#!/bin/bash

# Copyright (C) 2010 The Android Open Source Project
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

mkdir -p ../../../vendor/$MANUFACTURER/$DEVICE/proprietary

# DSP related libs and firmware
adb pull /system/lib/dsp/g729enc_sn.dll64P ../../../vendor/bn/encore/proprietary/g729enc_sn.dll64P
adb pull /system/lib/dsp/g722dec_sn.dll64P ../../../vendor/bn/encore/proprietary/g722dec_sn.dll64P
adb pull /system/lib/dsp/wbamrenc_sn.dll64P ../../../vendor/bn/encore/proprietary/wbamrenc_sn.dll64P
adb pull /system/lib/dsp/mp3dec_sn.dll64P ../../../vendor/bn/encore/proprietary/mp3dec_sn.dll64P
adb pull /system/lib/dsp/baseimage.map ../../../vendor/bn/encore/proprietary/baseimage.map
adb pull /system/lib/dsp/g726enc_sn.dll64P ../../../vendor/bn/encore/proprietary/g726enc_sn.dll64P
adb pull /system/lib/dsp/qosdyn_3430.dll64P ../../../vendor/bn/encore/proprietary/qosdyn_3430.dll64P
adb pull /system/lib/dsp/nbamrdec_sn.dll64P ../../../vendor/bn/encore/proprietary/nbamrdec_sn.dll64P
adb pull /system/lib/dsp/postprocessor_dualout.dll64P ../../../vendor/bn/encore/proprietary/postprocessor_dualout.dll64P
adb pull /system/lib/dsp/wbamrdec_sn.dll64P ../../../vendor/bn/encore/proprietary/wbamrdec_sn.dll64P
adb pull /system/lib/dsp/conversions.dll64P ../../../vendor/bn/encore/proprietary/conversions.dll64P
adb pull /system/lib/dsp/ddspbase_tiomap3430.dof64P ../../../vendor/bn/encore/proprietary/ddspbase_tiomap3430.dof64P
adb pull /system/lib/dsp/monitor_tiomap3430.dof64P ../../../vendor/bn/encore/proprietary/monitor_tiomap3430.dof64P
adb pull /system/lib/dsp/yuvconvert.l64p ../../../vendor/bn/encore/proprietary/yuvconvert.l64p
adb pull /system/lib/dsp/wmadec_sn.dll64P ../../../vendor/bn/encore/proprietary/wmadec_sn.dll64P
adb pull /system/lib/dsp/dctn_dyn.dll64P ../../../vendor/bn/encore/proprietary/dctn_dyn.dll64P
adb pull /system/lib/dsp/jpegenc_sn.dll64P ../../../vendor/bn/encore/proprietary/jpegenc_sn.dll64P
adb pull /system/lib/dsp/g722enc_sn.dll64P ../../../vendor/bn/encore/proprietary/g722enc_sn.dll64P
adb pull /system/lib/dsp/vpp_sn.dll64P ../../../vendor/bn/encore/proprietary/vpp_sn.dll64P
adb pull /system/lib/dsp/wmv9dec_sn.dll64P ../../../vendor/bn/encore/proprietary/wmv9dec_sn.dll64P
adb pull /system/lib/dsp/dfgm.dll64P ../../../vendor/bn/encore/proprietary/dfgm.dll64P
adb pull /system/lib/dsp/g729dec_sn.dll64P ../../../vendor/bn/encore/proprietary/g729dec_sn.dll64P
adb pull /system/lib/dsp/g711enc_sn.dll64P ../../../vendor/bn/encore/proprietary/g711enc_sn.dll64P
adb pull /system/lib/dsp/h264venc_sn.dll64P ../../../vendor/bn/encore/proprietary/h264venc_sn.dll64P
adb pull /system/lib/dsp/jpegdec_sn.dll64P ../../../vendor/bn/encore/proprietary/jpegdec_sn.dll64P
adb pull /system/lib/dsp/mp4vdec_sn.dll64P ../../../vendor/bn/encore/proprietary/mp4vdec_sn.dll64P
adb pull /system/lib/dsp/ipp_sn.dll64P ../../../vendor/bn/encore/proprietary/ipp_sn.dll64P
adb pull /system/lib/dsp/mpeg4aacenc_sn.dll64P ../../../vendor/bn/encore/proprietary/mpeg4aacenc_sn.dll64P
adb pull /system/lib/dsp/nbamrenc_sn.dll64P ../../../vendor/bn/encore/proprietary/nbamrenc_sn.dll64P
adb pull /system/lib/dsp/star.l64P ../../../vendor/bn/encore/proprietary/star.l64P
adb pull /system/lib/dsp/dynbase_tiomap3430.dof64P ../../../vendor/bn/encore/proprietary/dynbase_tiomap3430.dof64P
adb pull /system/lib/dsp/g726dec_sn.dll64P ../../../vendor/bn/encore/proprietary/g726dec_sn.dll64P
adb pull /system/lib/dsp/mpeg4aacdec_sn.dll64P ../../../vendor/bn/encore/proprietary/mpeg4aacdec_sn.dll64P
adb pull /system/lib/dsp/eenf_ti.l64P ../../../vendor/bn/encore/proprietary/eenf_ti.l64P
adb pull /system/lib/dsp/baseimage.dof ../../../vendor/bn/encore/proprietary/baseimage.dof
adb pull /system/lib/dsp/ringio.dll64P ../../../vendor/bn/encore/proprietary/ringio.dll64P
adb pull /system/lib/dsp/usn.dll64P ../../../vendor/bn/encore/proprietary/usn.dll64P
adb pull /system/lib/dsp/ilbcenc_sn.dll64P ../../../vendor/bn/encore/proprietary/ilbcenc_sn.dll64P
adb pull /system/lib/dsp/m4venc_sn.dll64P ../../../vendor/bn/encore/proprietary/m4venc_sn.dll64P
adb pull /system/lib/dsp/chromasuppress.l64p ../../../vendor/bn/encore/proprietary/chromasuppress.l64p
adb pull /system/lib/dsp/h264vdec_sn.dll64P ../../../vendor/bn/encore/proprietary/h264vdec_sn.dll64P
adb pull /system/lib/dsp/ilbcdec_sn.dll64P ../../../vendor/bn/encore/proprietary/ilbcdec_sn.dll64P
adb pull /system/lib/dsp/g711dec_sn.dll64P ../../../vendor/bn/encore/proprietary/g711dec_sn.dll64P


# DSP Codecs
adb pull /system/lib/libopencore_rtsp.so ../../../vendor/bn/encore/proprietary/libopencore_rtsp.so
adb pull /system/lib/libOMX.TI.G722.decode.so ../../../vendor/bn/encore/proprietary/libOMX.TI.G722.decode.so
adb pull /system/lib/libopencorehw.so ../../../vendor/bn/encore/proprietary/libopencorehw.so
adb pull /system/lib/libopencore_player.so ../../../vendor/bn/encore/proprietary/libopencore_player.so
adb pull /system/lib/libOMX.TI.G729.encode.so ../../../vendor/bn/encore/proprietary/libOMX.TI.G729.encode.so
adb pull /system/lib/libVendor_ti_omx.so ../../../vendor/bn/encore/proprietary/libVendor_ti_omx.so
adb pull /system/lib/libOMX.TI.AMR.decode.so ../../../vendor/bn/encore/proprietary/libOMX.TI.AMR.decode.so
adb pull /system/lib/libLCML.so ../../../vendor/bn/encore/proprietary/libLCML.so
adb pull /system/lib/libomx_amrenc_sharedlibrary.so ../../../vendor/bn/encore/proprietary/libomx_amrenc_sharedlibrary.so
adb pull /system/lib/libbridge.so ../../../vendor/bn/encore/proprietary/libbridge.so
adb pull /system/lib/libOMX.TI.MP3.decode.so ../../../vendor/bn/encore/proprietary/libOMX.TI.MP3.decode.so
adb pull /system/lib/libopencore_common.so ../../../vendor/bn/encore/proprietary/libopencore_common.so
adb pull /system/lib/libOMX.TI.JPEG.encoder.so ../../../vendor/bn/encore/proprietary/libOMX.TI.JPEG.encoder.so
adb pull /system/lib/libOMX.TI.WBAMR.decode.so ../../../vendor/bn/encore/proprietary/libOMX.TI.WBAMR.decode.so
adb pull /system/lib/libomx_m4vdec_sharedlibrary.so ../../../vendor/bn/encore/proprietary/libomx_m4vdec_sharedlibrary.so
adb pull /system/lib/libopencore_download.so ../../../vendor/bn/encore/proprietary/libopencore_download.so
adb pull /system/lib/libomx_aacdec_sharedlibrary.so ../../../vendor/bn/encore/proprietary/libomx_aacdec_sharedlibrary.so
adb pull /system/lib/libVendor_ti_omx_config_parser.so ../../../vendor/bn/encore/proprietary/libVendor_ti_omx_config_parser.so
adb pull /system/lib/libOMX.TI.G722.encode.so ../../../vendor/bn/encore/proprietary/libOMX.TI.G722.encode.so
adb pull /system/lib/libOMX.TI.Video.Decoder.so ../../../vendor/bn/encore/proprietary/libOMX.TI.Video.Decoder.so
adb pull /system/lib/libomx_mp3dec_sharedlibrary.so ../../../vendor/bn/encore/proprietary/libomx_mp3dec_sharedlibrary.so
adb pull /system/lib/libOMX.TI.AAC.encode.so ../../../vendor/bn/encore/proprietary/libOMX.TI.AAC.encode.so
adb pull /system/lib/libopencore_rtspreg.so ../../../vendor/bn/encore/proprietary/libopencore_rtspreg.so
adb pull /system/lib/libOMX.TI.720P.Decoder.so ../../../vendor/bn/encore/proprietary/libOMX.TI.720P.Decoder.so
adb pull /system/lib/libPERF.so ../../../vendor/bn/encore/proprietary/libPERF.so
adb pull /system/lib/libOMX.TI.ILBC.encode.so ../../../vendor/bn/encore/proprietary/libOMX.TI.ILBC.encode.so
adb pull /system/lib/libOMX.TI.AAC.decode.so ../../../vendor/bn/encore/proprietary/libOMX.TI.AAC.decode.so
adb pull /system/lib/libopencore_author.so ../../../vendor/bn/encore/proprietary/libopencore_author.so
adb pull /system/lib/libopencore_mp4localreg.so ../../../vendor/bn/encore/proprietary/libopencore_mp4localreg.so
adb pull /system/lib/libOMX.TI.G726.decode.so ../../../vendor/bn/encore/proprietary/libOMX.TI.G726.decode.so
adb pull /system/lib/libOMX.TI.VPP.so ../../../vendor/bn/encore/proprietary/libOMX.TI.VPP.so
adb pull /system/lib/libOMX.TI.JPEG.decoder.so ../../../vendor/bn/encore/proprietary/libOMX.TI.JPEG.decoder.so
adb pull /system/lib/libOMX.TI.Video.encoder.so ../../../vendor/bn/encore/proprietary/libOMX.TI.Video.encoder.so
adb pull /system/lib/libomx_amrdec_sharedlibrary.so ../../../vendor/bn/encore/proprietary/libomx_amrdec_sharedlibrary.so
adb pull /system/lib/libOMX_Core.so ../../../vendor/bn/encore/proprietary/libOMX_Core.so
adb pull /system/lib/libOMX.TI.G711.decode.so ../../../vendor/bn/encore/proprietary/libOMX.TI.G711.decode.so
adb pull /system/lib/libOMX.TI.WBAMR.encode.so ../../../vendor/bn/encore/proprietary/libOMX.TI.WBAMR.encode.so
adb pull /system/lib/libomx_avcdec_sharedlibrary.so ../../../vendor/bn/encore/proprietary/libomx_avcdec_sharedlibrary.so
adb pull /system/lib/libomap_mm_library_jni.so ../../../vendor/bn/encore/proprietary/libomap_mm_library_jni.so
adb pull /system/lib/libopencore_downloadreg.so ../../../vendor/bn/encore/proprietary/libopencore_downloadreg.so
adb pull /system/lib/libOMX.TI.G729.decode.so ../../../vendor/bn/encore/proprietary/libOMX.TI.G729.decode.so
adb pull /system/lib/libomx_sharedlibrary.so ../../../vendor/bn/encore/proprietary/libomx_sharedlibrary.so
adb pull /system/lib/libOMX.TI.WMA.decode.so ../../../vendor/bn/encore/proprietary/libOMX.TI.WMA.decode.so
adb pull /system/lib/libOMX.TI.ILBC.decode.so ../../../vendor/bn/encore/proprietary/libOMX.TI.ILBC.decode.so
adb pull /system/lib/libopencore_net_support.so ../../../vendor/bn/encore/proprietary/libopencore_net_support.so
adb pull /system/lib/libOMX.TI.G726.encode.so ../../../vendor/bn/encore/proprietary/libOMX.TI.G726.encode.so
adb pull /system/lib/libopencore_mp4local.so ../../../vendor/bn/encore/proprietary/libopencore_mp4local.so
adb pull /system/lib/libOMX.TI.G711.encode.so ../../../vendor/bn/encore/proprietary/libOMX.TI.G711.encode.so
adb pull /system/lib/libOMX.TI.AMR.encode.so ../../../vendor/bn/encore/proprietary/libOMX.TI.AMR.encode.so

# SGX SDK
adb pull /system/lib/libsrv_um.so.1.1.16.4061 ../../../vendor/bn/encore/proprietary/libsrv_um.so.1.1.16.4061
adb pull /system/lib/libIMGegl.so ../../../vendor/bn/encore/proprietary/libIMGegl.so
adb pull /system/lib/hw/gralloc.omap3.so.1.1.16.4061 ../../../vendor/bn/encore/proprietary/gralloc.omap3.so.1.1.16.4061
adb pull /system/lib/libpvrPVR2D_FLIPWSEGL.so.1.1.16.4061 ../../../vendor/bn/encore/proprietary/libpvrPVR2D_FLIPWSEGL.so.1.1.16.4061
adb pull /system/lib/libusc.so.1.1.16.4061 ../../../vendor/bn/encore/proprietary/libusc.so.1.1.16.4061
adb pull /system/lib/libglslcompiler.so.1.1.16.4061 ../../../vendor/bn/encore/proprietary/libglslcompiler.so.1.1.16.4061
adb pull /system/lib/libPVRScopeServices.so.1.1.16.4061 ../../../vendor/bn/encore/proprietary/libPVRScopeServices.so.1.1.16.4061
adb pull /system/lib/libpvrANDROID_WSEGL.so.1.1.16.4061 ../../../vendor/bn/encore/proprietary/libpvrANDROID_WSEGL.so.1.1.16.4061
adb pull /system/lib/libpvrPVR2D_FRONTWSEGL.so.1.1.16.4061 ../../../vendor/bn/encore/proprietary/libpvrPVR2D_FRONTWSEGL.so.1.1.16.4061
adb pull /system/lib/libOpenVG.so.1.1.16.4061 ../../../vendor/bn/encore/proprietary/libOpenVG.so.1.1.16.4061
adb pull /system/lib/libpvr2d.so.1.1.16.4061 ../../../vendor/bn/encore/proprietary/libpvr2d.so.1.1.16.4061
adb pull /system/lib/libsrv_init.so.1.1.16.4061 ../../../vendor/bn/encore/proprietary/libsrv_init.so.1.1.16.4061
adb pull /system/lib/libOpenVGU.so.1.1.16.4061 ../../../vendor/bn/encore/proprietary/libOpenVGU.so.1.1.16.4061
adb pull /system/lib/egl/libGLESv1_CM_POWERVR_SGX530_125.so.1.1.16.4061 ../../../vendor/bn/encore/proprietary/libGLESv1_CM_POWERVR_SGX530_125.so.1.1.16.4061
adb pull /system/lib/egl/libEGL_POWERVR_SGX530_125.so.1.1.16.4061 ../../../vendor/bn/encore/proprietary/libEGL_POWERVR_SGX530_125.so.1.1.16.4061
adb pull /system/lib/egl/libGLESv2_POWERVR_SGX530_125.so.1.1.16.4061 ../../../vendor/bn/encore/proprietary/libGLESv2_POWERVR_SGX530_125.so.1.1.16.4061
adb pull /system/lib/egl/libGLES_android.so ../../../vendor/bn/encore/proprietary/libGLES_android.so
adb pull /system/lib/egl/egl.cfg ../../../vendor/bn/encore/proprietary/egl.cfg
adb pull /system/bin/pvrsrvinit ../../../vendor/bn/encore/proprietary/pvrsrvinit
adb pull /system/etc/powervr.ini ../../../vendor/bn/encore/proprietary/powervr.ini

(cat << EOF) | sed s/__DEVICE__/$DEVICE/g | sed s/__MANUFACTURER__/$MANUFACTURER/g > ../../../vendor/$MANUFACTURER/$DEVICE/device-vendor-blobs.mk
# Copyright (C) 2010 The Android Open Source Project
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

# This file is generated by device/__MANUFACTURER__/__DEVICE__/extract-files.sh - DO NOT EDIT

PRODUCT_COPY_FILES += \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libopencore_rtsp.so:/system/lib/libopencore_rtsp.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.G722.decode.so:/system/lib/libOMX.TI.G722.decode.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libopencorehw.so:/system/lib/libopencorehw.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libopencore_player.so:/system/lib/libopencore_player.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.G729.encode.so:/system/lib/libOMX.TI.G729.encode.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libVendor_ti_omx.so:/system/lib/libVendor_ti_omx.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.AMR.decode.so:/system/lib/libOMX.TI.AMR.decode.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libLCML.so:/system/lib/libLCML.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libomx_amrenc_sharedlibrary.so:/system/lib/libomx_amrenc_sharedlibrary.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libbridge.so:/system/lib/libbridge.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.MP3.decode.so:/system/lib/libOMX.TI.MP3.decode.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libopencore_common.so:/system/lib/libopencore_common.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.JPEG.encoder.so:/system/lib/libOMX.TI.JPEG.encoder.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.WBAMR.decode.so:/system/lib/libOMX.TI.WBAMR.decode.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libomx_m4vdec_sharedlibrary.so:/system/lib/libomx_m4vdec_sharedlibrary.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libopencore_download.so:/system/lib/libopencore_download.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libomx_aacdec_sharedlibrary.so:/system/lib/libomx_aacdec_sharedlibrary.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libVendor_ti_omx_config_parser.so:/system/lib/libVendor_ti_omx_config_parser.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.G722.encode.so:/system/lib/libOMX.TI.G722.encode.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.Video.Decoder.so:/system/lib/libOMX.TI.Video.Decoder.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libomx_mp3dec_sharedlibrary.so:/system/lib/libomx_mp3dec_sharedlibrary.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.AAC.encode.so:/system/lib/libOMX.TI.AAC.encode.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libopencore_rtspreg.so:/system/lib/libopencore_rtspreg.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.720P.Decoder.so:/system/lib/libOMX.TI.720P.Decoder.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libPERF.so:/system/lib/libPERF.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.ILBC.encode.so:/system/lib/libOMX.TI.ILBC.encode.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.AAC.decode.so:/system/lib/libOMX.TI.AAC.decode.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libopencore_author.so:/system/lib/libopencore_author.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libopencore_mp4localreg.so:/system/lib/libopencore_mp4localreg.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.G726.decode.so:/system/lib/libOMX.TI.G726.decode.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.VPP.so:/system/lib/libOMX.TI.VPP.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.JPEG.decoder.so:/system/lib/libOMX.TI.JPEG.decoder.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.Video.encoder.so:/system/lib/libOMX.TI.Video.encoder.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libomx_amrdec_sharedlibrary.so:/system/lib/libomx_amrdec_sharedlibrary.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX_Core.so:/system/lib/libOMX_Core.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.G711.decode.so:/system/lib/libOMX.TI.G711.decode.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.WBAMR.encode.so:/system/lib/libOMX.TI.WBAMR.encode.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libomx_avcdec_sharedlibrary.so:/system/lib/libomx_avcdec_sharedlibrary.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libomap_mm_library_jni.so:/system/lib/libomap_mm_library_jni.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libopencore_downloadreg.so:/system/lib/libopencore_downloadreg.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.G729.decode.so:/system/lib/libOMX.TI.G729.decode.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libomx_sharedlibrary.so:/system/lib/libomx_sharedlibrary.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.WMA.decode.so:/system/lib/libOMX.TI.WMA.decode.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.ILBC.decode.so:/system/lib/libOMX.TI.ILBC.decode.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libopencore_net_support.so:/system/lib/libopencore_net_support.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.G726.encode.so:/system/lib/libOMX.TI.G726.encode.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libopencore_mp4local.so:/system/lib/libopencore_mp4local.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.G711.encode.so:/system/lib/libOMX.TI.G711.encode.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOMX.TI.AMR.encode.so:/system/lib/libOMX.TI.AMR.encode.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libsrv_um.so.1.1.16.4061:/system/lib/libsrv_um.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libIMGegl.so:/system/lib/libIMGegl.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/gralloc.omap3.so.1.1.16.4061:/system/lib/hw/gralloc.omap3.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libpvrPVR2D_FLIPWSEGL.so.1.1.16.4061:/system/lib/libpvrPVR2D_FLIPWSEGL.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libusc.so.1.1.16.4061:/system/lib/libusc.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libglslcompiler.so.1.1.16.4061:/system/lib/libglslcompiler.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libPVRScopeServices.so.1.1.16.4061:/system/lib/libPVRScopeServices.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libpvrANDROID_WSEGL.so.1.1.16.4061:/system/lib/libpvrANDROID_WSEGL.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libpvrPVR2D_FRONTWSEGL.so.1.1.16.4061:/system/lib/libpvrPVR2D_FRONTWSEGL.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOpenVG.so.1.1.16.4061:/system/lib/libOpenVG.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libpvr2d.so.1.1.16.4061:/system/lib/libpvr2d.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libsrv_init.so.1.1.16.4061:/system/lib/libsrv_init.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/g729enc_sn.dll64P:/system/lib/dsp/g729enc_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/g722dec_sn.dll64P:/system/lib/dsp/g722dec_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/wbamrenc_sn.dll64P:/system/lib/dsp/wbamrenc_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/mp3dec_sn.dll64P:/system/lib/dsp/mp3dec_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/baseimage.map:/system/lib/dsp/baseimage.map \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/g726enc_sn.dll64P:/system/lib/dsp/g726enc_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/qosdyn_3430.dll64P:/system/lib/dsp/qosdyn_3430.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/nbamrdec_sn.dll64P:/system/lib/dsp/nbamrdec_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/postprocessor_dualout.dll64P:/system/lib/dsp/postprocessor_dualout.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/wbamrdec_sn.dll64P:/system/lib/dsp/wbamrdec_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/conversions.dll64P:/system/lib/dsp/conversions.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/ddspbase_tiomap3430.dof64P:/system/lib/dsp/ddspbase_tiomap3430.dof64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/monitor_tiomap3430.dof64P:/system/lib/dsp/monitor_tiomap3430.dof64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/yuvconvert.l64p:/system/lib/dsp/yuvconvert.l64p \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/wmadec_sn.dll64P:/system/lib/dsp/wmadec_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/dctn_dyn.dll64P:/system/lib/dsp/dctn_dyn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/jpegenc_sn.dll64P:/system/lib/dsp/jpegenc_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/g722enc_sn.dll64P:/system/lib/dsp/g722enc_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/vpp_sn.dll64P:/system/lib/dsp/vpp_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/wmv9dec_sn.dll64P:/system/lib/dsp/wmv9dec_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/dfgm.dll64P:/system/lib/dsp/dfgm.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/g729dec_sn.dll64P:/system/lib/dsp/g729dec_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/g711enc_sn.dll64P:/system/lib/dsp/g711enc_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/h264venc_sn.dll64P:/system/lib/dsp/h264venc_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/jpegdec_sn.dll64P:/system/lib/dsp/jpegdec_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/mp4vdec_sn.dll64P:/system/lib/dsp/mp4vdec_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/ipp_sn.dll64P:/system/lib/dsp/ipp_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/mpeg4aacenc_sn.dll64P:/system/lib/dsp/mpeg4aacenc_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/nbamrenc_sn.dll64P:/system/lib/dsp/nbamrenc_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/star.l64P:/system/lib/dsp/star.l64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/dynbase_tiomap3430.dof64P:/system/lib/dsp/dynbase_tiomap3430.dof64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/g726dec_sn.dll64P:/system/lib/dsp/g726dec_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/mpeg4aacdec_sn.dll64P:/system/lib/dsp/mpeg4aacdec_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/eenf_ti.l64P:/system/lib/dsp/eenf_ti.l64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/baseimage.dof:/system/lib/dsp/baseimage.dof \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/ringio.dll64P:/system/lib/dsp/ringio.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/usn.dll64P:/system/lib/dsp/usn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/ilbcenc_sn.dll64P:/system/lib/dsp/ilbcenc_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/m4venc_sn.dll64P:/system/lib/dsp/m4venc_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/chromasuppress.l64p:/system/lib/dsp/chromasuppress.l64p \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/h264vdec_sn.dll64P:/system/lib/dsp/h264vdec_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/ilbcdec_sn.dll64P:/system/lib/dsp/ilbcdec_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/g711dec_sn.dll64P:/system/lib/dsp/g711dec_sn.dll64P \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libOpenVGU.so.1.1.16.4061:/system/lib/libOpenVGU.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libEGL_POWERVR_SGX530_125.so.1.1.16.4061:/system/lib/egl/libEGL_POWERVR_SGX530_125.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libGLESv2_POWERVR_SGX530_125.so.1.1.16.4061:/system/lib/egl/libGLESv2_POWERVR_SGX530_125.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libGLES_android.so:/system/lib/egl/libGLES_android.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/egl.cfg:/system/lib/egl/egl.cfg \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/libGLESv1_CM_POWERVR_SGX530_125.so.1.1.16.4061:/system/lib/egl/libGLESv1_CM_POWERVR_SGX530_125.so \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/pvrsrvinit:/system/bin/pvrsrvinit \\
    vendor/__MANUFACTURER__/__DEVICE__/proprietary/powervr.ini:/system/etc/powervr.ini \\
EOF


./setup-makefiles.sh

