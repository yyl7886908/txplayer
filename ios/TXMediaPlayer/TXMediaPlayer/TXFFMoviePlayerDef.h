/*
 * TXFFMoviePlayerDef.h
 *
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
 *
 * This file is part of TXPlayer.
 *
 * TXPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * TXPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with TXPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#import <Foundation/Foundation.h>
#include "txplayer/ios/txplayer_ios.h"



struct TXSize {
    NSInteger width;
    NSInteger height;
};
typedef struct TXSize TXSize;

CG_INLINE TXSize
TXSizeMake(NSInteger width, NSInteger height)
{
    TXSize size;
    size.width = width;
    size.height = height;
    return size;
}



struct TXSampleAspectRatio {
    NSInteger numerator;
    NSInteger denominator;
};
typedef struct TXSampleAspectRatio TXSampleAspectRatio;

CG_INLINE TXSampleAspectRatio
TXSampleAspectRatioMake(NSInteger numerator, NSInteger denominator)
{
    TXSampleAspectRatio sampleAspectRatio;
    sampleAspectRatio.numerator = numerator;
    sampleAspectRatio.denominator = denominator;
    return sampleAspectRatio;
}



@interface TXFFMoviePlayerMessage : NSObject {
@public
    AVMessage _msg;
}
@end


@interface TXFFMoviePlayerMessagePool : NSObject

- (TXFFMoviePlayerMessagePool *)init;
- (TXFFMoviePlayerMessage *) obtain;
- (void) recycle:(TXFFMoviePlayerMessage *)msg;

@end
