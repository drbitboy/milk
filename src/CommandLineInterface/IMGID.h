/**
 * @file    IMGID.h
 * @brief   Image identifying structure
 *
 */

#ifndef IMGID_H
#define IMGID_H

#include "ImageStreamIO/ImageStreamIO.h"

// image identifier type avoids name-resolving imageID multiple times
// provides quick and convenient access to data and metadata pointers
// Pass this as argument to functions to have both input-by-ID (ID>-1)
// and input-by-name (ID=-1).
typedef struct
{
    imageID ID; // -1 if not resolved

    // increments when image created, used to check if re-resolving needed
    int64_t createcnt;

    // used to resolve if needed
    char            name[STRINGMAXLEN_IMAGE_NAME];

    // image content, data and metadata
    IMAGE          *im;
    // md points at im.md
    IMAGE_METADATA *md;

    // Requested image params
    // Used to create image or test if existing image matches
    // These fields do not always match the image content
    //
    uint8_t  datatype;
    int      naxis;
    uint32_t size[3];
    int      shared;
    // number of keywords
    int      NBkw;

    // fast circular buffer size
    int CBsize;

} IMGID;

#endif
