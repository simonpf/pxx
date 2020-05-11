from setuptools import setup, find_packages
from os import path
import shutil

with open(path.join("@CMAKE_SOURCE_DIR@", 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

try:
    lib_path = path.join("@CMAKE_BINARY_DIR@", "src", "_pxx.so")
    shutil.copy(lib_path, "_pxx.so")
except:
    raise Exception("Could not find the _pxx.so shared library, which is required for "
                    " the pxx Python module. Please make sure the installation was "
                    "successful.")

setup(
    name='pxx',
    version='@VERSION@',
    description='Automatic generation of Python modules for C++ code.',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/simonpf/pxx',
    author='Simon Pfreundschuh',
    author_email='simon.pfreundschuh@chalmers.se',
    install_requires=[],
    packages=["pxx"],
    package_data={"pxx" : ["_pxx.so"]},
    python_requires='>=3.6',
    project_urls={
        'Source': 'https://github.com/simonpf/pxx/',
    })
