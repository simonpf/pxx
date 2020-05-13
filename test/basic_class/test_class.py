import pytest
import sys
import os

try:
    sys.path.append(os.path.dirname(__file__))
except:
    sys.path.append(os.path.dirname("."))

from basic_class import TestClass

def test_basic_class():
    """
    Test calling of member function and access to public members.
    """
    tc = TestClass()
    assert(tc.get_string() == "hello")
    assert(tc.get_int() == 42)
    assert(tc.public_member_1 == 1)
    assert(tc.public_member_2 == 2)


