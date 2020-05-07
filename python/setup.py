from setuptools import setup, find_packages
from os import path

here = path.abspath(path.dirname(__file__))
with open(path.join(here, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

setup(
    name='pxx',
    version=@@VERSION@@,
    description='Automatic generation of Python modules for C++ code.',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/simonpf/pxx',
    author='Simon Pfreundschuh',
    author_email='simon.pfreundschuh@chalmers.se',
    install_requires=[],
    packages=["camels"],
    python_requires='>=3.6',
    project_urls={
        'Source': 'https://github.com/simonpf/camels/',
    })
