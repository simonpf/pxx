import pytest
import sys
import os

try:
    sys.path.append(os.path.dirname(__file__))
except:
    sys.path.append(os.path.dirname("."))

from basic_class import TestClass

def test_basic_class():
    tc = TestClass()
    tc = TestClass(10)


