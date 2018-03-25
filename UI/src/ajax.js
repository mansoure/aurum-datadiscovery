var httpRequest;
var callback;

export function makeRequest(query, newCallback) {
  callback = newCallback;
  var url = "http://127.0.0.1:5000/query/" + query;

  httpRequest = new XMLHttpRequest();

  if (!httpRequest) {
    alert('Giving up :( Cannot create an XMLHTTP instance');
    return false;
  }
  httpRequest.onreadystatechange = ignoreErr;
  httpRequest.open('GET', url);
  httpRequest.send();
}

export function makeExport(newCallback) {
  callback = newCallback;
  var url = "http://127.0.0.1:5000/export";

  httpRequest = new XMLHttpRequest();

  if (!httpRequest) {
    alert('Giving up :( Cannot create an XMLHTTP instance');
    return false;
  }
  httpRequest.onreadystatechange = ignoreErr;
  httpRequest.open('GET', url);
  httpRequest.send();
}

export function makeConvert(identifier, newCallback) {
  callback = newCallback;
  var url = "http://127.0.0.1:5000/convert/" + identifier;

  httpRequest = new XMLHttpRequest();

  if (!httpRequest) {
    alert('Giving up :( Cannot create an XMLHTTP instance');
    return false;
  }
  httpRequest.onreadystatechange = ignoreErr;
  httpRequest.open('GET', url);
  httpRequest.send();
}

export function makeInspect(identifier, newCallback) {
  callback = newCallback;
  var url = "http://127.0.0.1:5000/inspect/" + identifier;

  httpRequest = new XMLHttpRequest();

  if (!httpRequest) {
    alert('Giving up :( Cannot create an XMLHTTP instance');
    return false;
  }
  httpRequest.onreadystatechange = ignoreErr;
  httpRequest.open('GET', url);
  httpRequest.send();
}

export function makeClean(source, target, newCallback) {
  callback = newCallback;
  var url = "http://127.0.0.1:5000/clean/" + source + "/" + target;

  httpRequest = new XMLHttpRequest();

  if (!httpRequest) {
    alert('Giving up :( Cannot create an XMLHTTP instance');
    return false;
  }
  httpRequest.onreadystatechange = ignoreErr;
  httpRequest.open('GET', url);
  httpRequest.send();
}


function ignoreErr() {
  if (httpRequest.readyState === XMLHttpRequest.DONE) {
    if (httpRequest.status === 200) {
      callback(httpRequest);
    } else {
      // console.log('There was a problem with the request.');
      // console.log(httpRequest);
    }
  }
}

