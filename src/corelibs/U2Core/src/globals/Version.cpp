#include "Version.h"

// UGENE_VERSION must be supplied as a preprocessor directive
#ifndef UGENE_VERSION
#error UGENE_VERSION is not set!
#endif

namespace U2 {

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define UGENE_VERSION_STRING TOSTRING(UGENE_VERSION)
#define VERSION_DEV_SUFFIX "dev"

Version::Version() {
    major = minor = patch = 0;
    debug = false;
    text = "unknown";
}

Version Version::parseVersion(const QString& text) {
    Version v;
    v.text = text;

    //parse sub-numbers and suffix
    int versionType = 0;
    QString currentNum;
    int textPos = 0;
    for (; textPos < v.text.length(); textPos++ ) {
        QChar c = v.text.at(textPos);
        if (c.isNumber()) {
            currentNum += c;
        } else {
            bool ok;
            int val = currentNum.toInt(&ok);
            if (!ok) {
                break;
            }
            if (versionType == 0) {
                v.major = val;
            } else if (versionType == 1) {
                v.minor = val;
            } else {
                v.patch = val;
                break;
            }
            versionType++;
            currentNum.clear();
        }
    }
    v.suffix = v.text.mid(textPos);
    v.isDevVersion = v.suffix.contains(VERSION_DEV_SUFFIX);
    
#ifdef _DEBUG
    v.debug = true;
#else
    v.debug = false;
#endif

    return v;
}

Version Version::ugeneVersion() {
    return parseVersion(UGENE_VERSION_STRING);
}

Version Version::qtVersion() {
    return parseVersion(QT_VERSION_STR);
}


bool Version::operator  >  (const Version& v) const {
    return v < *this;   
}

bool Version::operator  >= (const Version& v) const {
    return v <= *this;
}

bool Version::operator  <  (const Version& v) const {
    if (v.major != major) {
        return v.major > major;
    }

    if (v.minor != minor) {
        return v.minor > minor;
    }

    if (v.patch != patch) {
        return v.patch > patch;
    }

    return false;

}

bool Version::operator  <= (const Version& v) const {
    return *this < v || *this == v;
}

bool Version::operator  == (const Version& v) const {
    return major == v.major && minor == v.minor && patch == v.patch;
}


} //namespace

