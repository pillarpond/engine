// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/image_filter.h"

#include "flutter/tonic/dart_args.h"
#include "flutter/tonic/dart_binding_macros.h"
#include "lib/tonic/converter/dart_converter.h"
#include "flutter/tonic/dart_library_natives.h"
#include "third_party/skia/include/effects/SkBlurImageFilter.h"
#include "third_party/skia/include/effects/SkImageSource.h"
#include "third_party/skia/include/effects/SkPictureImageFilter.h"

namespace blink {

static void ImageFilter_constructor(Dart_NativeArguments args) {
  DartCallConstructor(&ImageFilter::Create, args);
}

IMPLEMENT_WRAPPERTYPEINFO(ui, ImageFilter);

#define FOR_EACH_BINDING(V) \
  V(ImageFilter, initImage) \
  V(ImageFilter, initPicture) \
  V(ImageFilter, initBlur)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void ImageFilter::RegisterNatives(DartLibraryNatives* natives) {
  natives->Register({
    { "ImageFilter_constructor", ImageFilter_constructor, 1, true },
FOR_EACH_BINDING(DART_REGISTER_NATIVE)
  });
}

scoped_refptr<ImageFilter> ImageFilter::Create() {
  return new ImageFilter();
}

ImageFilter::ImageFilter() {
}

ImageFilter::~ImageFilter() {
}

void ImageFilter::initImage(CanvasImage* image) {
  filter_ = SkImageSource::Make(image->image());
}

void ImageFilter::initPicture(Picture* picture) {
  filter_ = SkPictureImageFilter::Make(picture->picture());
}

void ImageFilter::initBlur(double sigma_x, double sigma_y) {
  filter_ = SkBlurImageFilter::Make(sigma_x, sigma_y, nullptr);
}

} // namespace blink
