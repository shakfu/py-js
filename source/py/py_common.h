/* py_common.h */

#ifndef PY_COMMON_H
#define PY_COMMON_H

/* conditional includes */
#if defined(__APPLE__) && (defined(PY_STATIC_EXT) || defined(PY_SHARED_PKG))
#include <CoreFoundation/CoreFoundation.h>
#include <libgen.h>
#endif


#if defined(__APPLE__) && defined(PY_STATIC_EXT)
void py_init_osx_set_home_static_ext(void)
{
    // sets python_home to <bundle>/Resources folder

    wchar_t* python_home;

    CFURLRef resources_url;
    CFURLRef resources_abs_url;
    CFStringRef resources_str;
    const char* resources_path;

    // Look for a bundle using its using global bundle ref
    resources_url = CFBundleCopyResourcesDirectoryURL(py_global_bundle);
    resources_abs_url = CFURLCopyAbsoluteURL(resources_url);
    resources_str = CFURLCopyFileSystemPath(resources_abs_url,
                                            kCFURLPOSIXPathStyle);
    resources_path = CFStringGetCStringPtr(resources_str,
                                           kCFStringEncodingUTF8);

    python_home = Py_DecodeLocale(resources_path, NULL);

    CFRelease(resources_str);
    CFRelease(resources_abs_url);
    CFRelease(resources_url);

    post("py resources_path: %s", resources_path);

    if (python_home == NULL) {
        error("unable to set python_home");
        return;
    }
    Py_SetPythonHome(python_home);
}
#endif


#if defined(__APPLE__) && defined(PY_SHARED_PKG)
void py_init_osx_set_home_shared_pkg(void)
{
    // sets python_home to <package>/support/pythonX.Y folder

    wchar_t* python_home;

    CFURLRef bundle_url;
    CFURLRef bundle_abs_url;
    CFStringRef bundle_str;
    const char* bundle_path;

    const char* relative_path = "support/python" PY_VER;
    CFStringRef relative_path_str;
    CFURLRef externals_url;
    CFURLRef package_url;
    CFURLRef py_home_url;
    CFStringRef py_home_str;
    const char* py_home_path;

    // get self bundle path
    bundle_url = CFBundleCopyBundleURL(py_global_bundle);
    bundle_abs_url = CFURLCopyAbsoluteURL(bundle_url);
    bundle_str = CFURLCopyFileSystemPath(bundle_abs_url, kCFURLPOSIXPathStyle);
    bundle_path = CFStringGetCStringPtr(bundle_str, kCFStringEncodingUTF8);

    // get the absolute path of the <package>/support/pythonX.Y directory in a
    // package
    externals_url = CFURLCreateCopyDeletingLastPathComponent(
        kCFAllocatorDefault, bundle_abs_url);
    package_url = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault,
                                                           externals_url);
    relative_path_str = CFStringCreateWithCString(
        kCFAllocatorDefault, relative_path, kCFStringEncodingASCII);
    py_home_url = CFURLCreateCopyAppendingPathComponent(
        kCFAllocatorDefault, package_url, relative_path_str, FALSE);
    py_home_str = CFURLCopyFileSystemPath(py_home_url, kCFURLPOSIXPathStyle);
    py_home_path = CFStringGetCStringPtr(py_home_str, kCFStringEncodingUTF8);

    CFRelease(bundle_str);
    CFRelease(bundle_abs_url);
    CFRelease(bundle_url);

    CFRelease(py_home_str);
    CFRelease(py_home_url);
    CFRelease(relative_path_str);
    CFRelease(package_url);
    CFRelease(externals_url);

    post("py bundle_path: %s", bundle_path);

    post("py home path: %s", py_home_path);

    python_home = Py_DecodeLocale(py_home_path, NULL);

    if (python_home == NULL) {
        error("unable to set python_home");
        return;
    }
    Py_SetPythonHome(python_home);
}
#endif


#endif // PY_COMMON_H
