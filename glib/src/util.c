
#include <CoreFoundation/CoreFoundation.h>
#include <string.h>

#include <util.h>


#define RESOURCE_DIR_BUF_LEN 128


/*
 * given a relative URL, gives back the full URL to be used to open a file
 */
int sysdep_url_to_abs_url(char * restrict buf, size_t buf_len, const char * restrict url) {

    CFBundleRef mb = CFBundleGetMainBundle();
    CFURLRef mu = CFBundleCopyResourcesDirectoryURL(mb);
    CFStringRef ms = CFURLGetString(mu);
    char rbuf[RESOURCE_DIR_BUF_LEN];
    CFStringGetCString(ms, rbuf, sizeof(rbuf), kCFStringEncodingASCII);
    snprintf(buf, buf_len, "%s%s\n", rbuf, url);

    printf("%s\n", buf);

    return 0;
}



