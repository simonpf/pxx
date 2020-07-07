def __bootstrap__():
    import pkg_resources, imp
    global __bootstrap__
    file = pkg_resources.resource_filename("pxx", '_pxx.so')
    file_handle = open(file, "rb")
    module = imp.load_module("_pxx", file_handle, file, ("", "rb", imp.C_EXTENSION))
    globals().update(module.__dict__)
    del __bootstrap__
__bootstrap__()
