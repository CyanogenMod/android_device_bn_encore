#!/bin/sh

# Copyright (C) 2012 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

DEVICE=encore
MANUFACTURER=bn

BASE=../../../vendor/$MANUFACTURER/$DEVICE/proprietary

if [ $1 ]; then
    ZIPFILE=$1
else
    ZIPFILE=../../../${DEVICE}_update.zip
fi

if [ ! -f "$1" ]; then
    echo "Cannot find $ZIPFILE.  Try specifify the stock update.zip with $0 <zipfilename>"
    exit 1
fi

for FILE in `cat proprietary-files.txt`; do
    DIR=`dirname $FILE`
    if [ ! -d $BASE/$DIR ]; then
        mkdir -p $BASE/$DIR
    fi
    unzip -j -o $ZIPFILE system/$FILE -d $BASE/$DIR/
done

./setup-makefiles.sh
