from setuptools import setup, Extension
setup(name='joinpathselection',
      version='0.1',
      ext_modules=[Extension('_joinpathselection', sources=['JPSelection.cc','Reader.cc','Common.cc','Corpus.cc', 'joinpathselection.i'],
                             swig_opts=['-c++', '-py3'], extra_compile_args=['-O3', '-std=c++11'])],
      headers=['JPSelection.h','Reader.h','Common.h','Corpus.h',]
)
