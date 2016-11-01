# Add Docker apt repo gpg key
apt-key adv --keyserver hkp://pgp.mit.edu:80 --recv-keys \
    58118E89F3A912897C070ADBF76221572C52609D
echo "deb https://apt.dockerproject.org/repo ubuntu-trusty main" > \
    /etc/apt/sources.list.d/docker.list
apt-get update
apt-get install -y docker-engine

# Build a version of Jupyter Notebook with all Aurum's dependencies and API
# code included.
cd /vagrant
docker build -t aurum-notebook -f Dockerfile-demo-notebook .

# Check out and build a tweaked version of tmpnb which supports specifying the
# container network
cd ~
git clone https://github.com/datahuborg/tmpnb.git aurum-tmpnb
cd aurum-tmpnb
docker build -t aurum-tmpnb .

# Create a separate network for Jupyter notebooks to talk with elasticsearch.
# Block them from talking to anything else.
docker network create aurum
AURUM_LINK_NAME=br-$(docker network ls -q -f name=aurum)
iptables -I FORWARD 2 -i $AURUM_LINK_NAME -o eth0 -j REJECT

# Create a separate data container to persist elasticsearch's data.
docker run -d \
    --name elasticsearch \
    --net=aurum \
    -v /aurum-data/mass:/usr/share/elasticsearch/data \
    -p 127.0.0.1:9200:9200 \
    -p 127.0.0.1:9300:9300 \
    elasticsearch:2.3.5

# Set up tmpnb and its proxy.
export TOKEN=$( head -c 30 /dev/urandom | xxd -p )
export SITE_KEY=""
export SECRET_KEY=""
echo $TOKEN > ~/proxy_auth_token
echo $SITE_KEY > ~/site_key
echo $SECRET_KEY > ~/secret_key

# Create a web proxy
docker run -d \
    --net=host \
    -e CONFIGPROXY_AUTH_TOKEN=$TOKEN \
    -u root \
    --name=proxy jupyter/configurable-http-proxy \
    --default-target http://127.0.0.1:9999 \
    --port 80 \
    --api-port 8001

# Create tmpnb, which manages the pool of notebook servers
# - Max 10 concurrent notebooks
# - Max 512 MB RAM to each
# - Max 12% of CPU to each
# - Kill sessions longer than 1 hour
# - Kill sessions idle longer than 30 minutes
# - Check lifetime and idleness every 10 minutes
docker run -d \
    --net=host \
    -e CONFIGPROXY_AUTH_TOKEN=$TOKEN \
    --name=tmpnb \
    -v /var/run/docker.sock:/docker.sock \
    aurum-tmpnb \
    python orchestrate.py --image='aurum-notebook' \
        --container_network='aurum' \
        --recaptcha_key=$SITE_KEY \
        --recaptcha_secret=$SECRET_KEY \
        --redirect_uri='/notebooks/Aurum%20Demo%20-%20Massachusetts%20Open%20Data.ipynb' \
        --pool_size=10 \
        --mem-limit='512m' \
        --cpu_shares=12000 \
        --cull-period=600 \
        --cull-max=3600 \
        --cull-timeout=1800 \
        --command='start-notebook.sh \
            "--NotebookApp.base_url={base_path} \
            --ip=0.0.0.0 \
            --port={port} \
            --NotebookApp.trust_xheaders=True"'
