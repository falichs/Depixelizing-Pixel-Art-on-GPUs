cmake_minimum_required (VERSION 2.6)

project(ExternalLibraries)
############################
#GLFW
############################
add_subdirectory ("${PROJECT_SOURCE_DIR}/glfw-3.1.1")

############################
#GLEW
############################
add_subdirectory ("${PROJECT_SOURCE_DIR}/glew-1.13.0/build/cmake")

############################
#FreeImage
############################
set(freeImage_SRC_DIR "${PROJECT_SOURCE_DIR}/FreeImage/Source")
#add zlib
set(zlib_SRC_DIR "${freeImage_SRC_DIR}/ZLib")
FILE(GLOB zlib_SRC_FILES ${zlib_SRC_DIR}/*.c)
FILE(GLOB zlib_HEADER_FILES ${zlib_SRC_DIR}/*.h)
add_library(zlib ${zlib_SRC_FILES} ${zlib_HEADER_FILES})
target_compile_definitions(zlib PRIVATE _LIB _CRT_SECURE_NO_DEPRECATE _CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS)

#add libJPEG
set(libJPEG_SRC_DIR "${freeImage_SRC_DIR}/LibJPEG")
set(libJPEG_SRC_FILES
"${libJPEG_SRC_DIR}/jaricom.c"
"${libJPEG_SRC_DIR}/jcapimin.c"
"${libJPEG_SRC_DIR}/jcapistd.c"
"${libJPEG_SRC_DIR}/jcarith.c"
"${libJPEG_SRC_DIR}/jccoefct.c"
"${libJPEG_SRC_DIR}/jccolor.c"
"${libJPEG_SRC_DIR}/jcdctmgr.c"
"${libJPEG_SRC_DIR}/jchuff.c"
"${libJPEG_SRC_DIR}/jcinit.c"
"${libJPEG_SRC_DIR}/jcmainct.c"
"${libJPEG_SRC_DIR}/jcmarker.c"
"${libJPEG_SRC_DIR}/jcmaster.c"
"${libJPEG_SRC_DIR}/jcomapi.c"
"${libJPEG_SRC_DIR}/jcparam.c"
"${libJPEG_SRC_DIR}/jcprepct.c"
"${libJPEG_SRC_DIR}/jcsample.c"
"${libJPEG_SRC_DIR}/jctrans.c"
"${libJPEG_SRC_DIR}/jdapimin.c"
"${libJPEG_SRC_DIR}/jdapistd.c"
"${libJPEG_SRC_DIR}/jdarith.c"
"${libJPEG_SRC_DIR}/jdatadst.c"
"${libJPEG_SRC_DIR}/jdatasrc.c"
"${libJPEG_SRC_DIR}/jdcoefct.c"
"${libJPEG_SRC_DIR}/jdcolor.c"
"${libJPEG_SRC_DIR}/jddctmgr.c"
"${libJPEG_SRC_DIR}/jdhuff.c"
"${libJPEG_SRC_DIR}/jdinput.c"
"${libJPEG_SRC_DIR}/jdmainct.c"
"${libJPEG_SRC_DIR}/jdmarker.c"
"${libJPEG_SRC_DIR}/jdmaster.c"
"${libJPEG_SRC_DIR}/jdmerge.c"
"${libJPEG_SRC_DIR}/jdpostct.c"
"${libJPEG_SRC_DIR}/jdsample.c"
"${libJPEG_SRC_DIR}/jdtrans.c"
"${libJPEG_SRC_DIR}/jerror.c"
"${libJPEG_SRC_DIR}/jfdctflt.c"
"${libJPEG_SRC_DIR}/jfdctfst.c"
"${libJPEG_SRC_DIR}/jfdctint.c"
"${libJPEG_SRC_DIR}/jidctflt.c"
"${libJPEG_SRC_DIR}/jidctfst.c"
"${libJPEG_SRC_DIR}/jidctint.c"
"${libJPEG_SRC_DIR}/jmemmgr.c"
"${libJPEG_SRC_DIR}/jmemnobs.c"
"${libJPEG_SRC_DIR}/jquant1.c"
"${libJPEG_SRC_DIR}/jquant2.c"
"${libJPEG_SRC_DIR}/jutils.c"
"${libJPEG_SRC_DIR}/transupp.c"
)
FILE(GLOB libJPEG_HEADER_FILES ${libJPEG_SRC_DIR}/*.h)
add_library(libJPEG ${libJPEG_SRC_FILES} ${libJPEG_HEADER_FILES})
target_link_libraries(libJPEG zlib)
target_compile_definitions(libJPEG PRIVATE _LIB _CRT_SECURE_NO_DEPRECATE)

#add libJXR
set(libJXR_SRC_DIR "${freeImage_SRC_DIR}/LibJXR")
set(libJXR_SRC_FILES 
"${libJXR_SRC_DIR}/image/decode/decode.c"
"${libJXR_SRC_DIR}/image/decode/JXRTranscode.c"
"${libJXR_SRC_DIR}/image/decode/postprocess.c"
"${libJXR_SRC_DIR}/image/decode/segdec.c"
"${libJXR_SRC_DIR}/image/decode/strdec.c"
"${libJXR_SRC_DIR}/image/decode/strdec_x86.c"
"${libJXR_SRC_DIR}/image/decode/strInvTransform.c"
"${libJXR_SRC_DIR}/image/decode/strPredQuantDec.c"
"${libJXR_SRC_DIR}/image/encode/encode.c"
"${libJXR_SRC_DIR}/image/encode/segenc.c"
"${libJXR_SRC_DIR}/image/encode/strenc.c"
"${libJXR_SRC_DIR}/image/encode/strenc_x86.c"
"${libJXR_SRC_DIR}/image/encode/strFwdTransform.c"
"${libJXR_SRC_DIR}/image/encode/strPredQuantEnc.c"
"${libJXR_SRC_DIR}/image/sys/adapthuff.c"
"${libJXR_SRC_DIR}/image/sys/image.c"
"${libJXR_SRC_DIR}/image/sys/strcodec.c"
"${libJXR_SRC_DIR}/image/sys/strPredQuant.c"
"${libJXR_SRC_DIR}/image/sys/strTransform.c"
"${libJXR_SRC_DIR}/jxrgluelib/JXRGlue.c"
"${libJXR_SRC_DIR}/jxrgluelib/JXRGlueJxr.c"
"${libJXR_SRC_DIR}/jxrgluelib/JXRGluePFC.c"
"${libJXR_SRC_DIR}/jxrgluelib/JXRMeta.c"
"${libJXR_SRC_DIR}/image/decode/decode.h"
"${libJXR_SRC_DIR}/image/encode/encode.h"
"${libJXR_SRC_DIR}/image/sys/ansi.h"
"${libJXR_SRC_DIR}/image/sys/common.h"
"${libJXR_SRC_DIR}/image/sys/perfTimer.h"
"${libJXR_SRC_DIR}/image/sys/strcodec.h"
"${libJXR_SRC_DIR}/image/sys/strTransform.h"
"${libJXR_SRC_DIR}/image/sys/windowsmediaphoto.h"
"${libJXR_SRC_DIR}/image/sys/xplatform_image.h"
"${libJXR_SRC_DIR}/image/x86/x86.h"
"${libJXR_SRC_DIR}/common/include/guiddef.h"
"${libJXR_SRC_DIR}/common/include/wmsal.h"
"${libJXR_SRC_DIR}/common/include/wmspecstring.h"
"${libJXR_SRC_DIR}/common/include/wmspecstrings_adt.h"
"${libJXR_SRC_DIR}/common/include/wmspecstrings_strict.h"
"${libJXR_SRC_DIR}/common/include/wmspecstrings_undef.h"
"${libJXR_SRC_DIR}/jxrgluelib/JXRGlue.h"
"${libJXR_SRC_DIR}/jxrgluelib/JXRMeta.h"
)
include_directories("${libJXR_SRC_DIR}/jxrgluelib" "${libJXR_SRC_DIR}/image/sys")
add_library(libJXR ${libJXR_SRC_FILES})
target_compile_definitions(libJXR PRIVATE _LIB DISABLE_PERF_MEASUREMENT)

#add LibOpenJPEG
set(libOpenJPEG_SRC_DIR "${freeImage_SRC_DIR}/LibOpenJPEG")
set(libOpenJPEG_SRC_FILES
"${libOpenJPEG_SRC_DIR}/bio.h"
"${libOpenJPEG_SRC_DIR}/cio.h"
"${libOpenJPEG_SRC_DIR}/dwt.h"
"${libOpenJPEG_SRC_DIR}/event.h"
"${libOpenJPEG_SRC_DIR}/function_list.h"
"${libOpenJPEG_SRC_DIR}/image.h"
"${libOpenJPEG_SRC_DIR}/indexbox_manager.h"
"${libOpenJPEG_SRC_DIR}/invert.h"
"${libOpenJPEG_SRC_DIR}/j2k.h"
"${libOpenJPEG_SRC_DIR}/jp2.h"
"${libOpenJPEG_SRC_DIR}/mct.h"
"${libOpenJPEG_SRC_DIR}/mqc.h"
"${libOpenJPEG_SRC_DIR}/openjpeg.h"
"${libOpenJPEG_SRC_DIR}/opj_clock.h"
"${libOpenJPEG_SRC_DIR}/opj_config.h"
"${libOpenJPEG_SRC_DIR}/opj_config_private.h"
"${libOpenJPEG_SRC_DIR}/opj_includes.h"
"${libOpenJPEG_SRC_DIR}/opj_intmath.h"
"${libOpenJPEG_SRC_DIR}/opj_inttypes.h"
"${libOpenJPEG_SRC_DIR}/opj_malloc.h"
"${libOpenJPEG_SRC_DIR}/opj_stdint.h"
"${libOpenJPEG_SRC_DIR}/pi.h"
"${libOpenJPEG_SRC_DIR}/raw.h"
"${libOpenJPEG_SRC_DIR}/t1.h"
"${libOpenJPEG_SRC_DIR}/t1_luts.h"
"${libOpenJPEG_SRC_DIR}/t2.h"
"${libOpenJPEG_SRC_DIR}/tcd.h"
"${libOpenJPEG_SRC_DIR}/tgt.h"
"${libOpenJPEG_SRC_DIR}/bio.c"
"${libOpenJPEG_SRC_DIR}/cio.c"
"${libOpenJPEG_SRC_DIR}/dwt.c"
"${libOpenJPEG_SRC_DIR}/event.c"
"${libOpenJPEG_SRC_DIR}/function_list.c"
"${libOpenJPEG_SRC_DIR}/image.c"
"${libOpenJPEG_SRC_DIR}/invert.c"
"${libOpenJPEG_SRC_DIR}/j2k.c"
"${libOpenJPEG_SRC_DIR}/jp2.c"
"${libOpenJPEG_SRC_DIR}/mct.c"
"${libOpenJPEG_SRC_DIR}/mqc.c"
"${libOpenJPEG_SRC_DIR}/openjpeg.c"
"${libOpenJPEG_SRC_DIR}/opj_clock.c"
"${libOpenJPEG_SRC_DIR}/pi.c"
"${libOpenJPEG_SRC_DIR}/raw.c"
"${libOpenJPEG_SRC_DIR}/t1.c"
"${libOpenJPEG_SRC_DIR}/t2.c"
"${libOpenJPEG_SRC_DIR}/tcd.c"
"${libOpenJPEG_SRC_DIR}/tgt.c"
)
add_library(libOpenJPEG ${libOpenJPEG_SRC_FILES})
target_link_libraries(libOpenJPEG zlib)
target_compile_definitions(libOpenJPEG PRIVATE _LIB OPJ_STATIC _CRT_SECURE_NO_DEPRECATE)

#add libPNG
set(libPNG_SRC_DIR "${freeImage_SRC_DIR}/LibPNG")
set(libPNG_SRC_FILES
"${libPNG_SRC_DIR}/png.c"
"${libPNG_SRC_DIR}/pngerror.c"
"${libPNG_SRC_DIR}/pngget.c"
"${libPNG_SRC_DIR}/pngmem.c"
"${libPNG_SRC_DIR}/pngpread.c"
"${libPNG_SRC_DIR}/pngread.c"
"${libPNG_SRC_DIR}/pngrio.c"
"${libPNG_SRC_DIR}/pngrtran.c"
"${libPNG_SRC_DIR}/pngrutil.c"
"${libPNG_SRC_DIR}/pngset.c"
"${libPNG_SRC_DIR}/pngtrans.c"
"${libPNG_SRC_DIR}/pngwio.c"
"${libPNG_SRC_DIR}/pngwrite.c"
"${libPNG_SRC_DIR}/pngwtran.c"
"${libPNG_SRC_DIR}/pngwutil.c"
"${libPNG_SRC_DIR}/png.h"
"${libPNG_SRC_DIR}/pngconf.h"
"${libPNG_SRC_DIR}/pngdebug.h"
"${libPNG_SRC_DIR}/pnginfo.h"
"${libPNG_SRC_DIR}/pnglibconf.h"
"${libPNG_SRC_DIR}/pngpriv.h"
"${libPNG_SRC_DIR}/pngstruct.h"
)
include_directories(${zlib_SRC_DIR})
add_library(libPNG ${libPNG_SRC_FILES})
target_link_libraries(libPNG zlib)
target_compile_definitions(libPNG PRIVATE _LIB _CRT_SECURE_NO_DEPRECATE)

#add libRAW
set(libRAW_SRC_DIR "${freeImage_SRC_DIR}/LibRawLite")
set(libRAW_SRC_FILES 
"${libRAW_SRC_DIR}/internal/dcraw_common.cpp"
"${libRAW_SRC_DIR}/internal/dcraw_fileio.cpp"
"${libRAW_SRC_DIR}/internal/demosaic_packs.cpp"
"${libRAW_SRC_DIR}/src/libraw_c_api.cpp"
"${libRAW_SRC_DIR}/src/libraw_cxx.cpp"
"${libRAW_SRC_DIR}/src/libraw_datastream.cpp"
"${libRAW_SRC_DIR}/internal/defines.h"
"${libRAW_SRC_DIR}/internal/libraw_internal_funcs.h"
"${libRAW_SRC_DIR}/internal/var_defines.h"
"${libRAW_SRC_DIR}/libraw/libraw.h"
"${libRAW_SRC_DIR}/libraw/libraw_alloc.h"
"${libRAW_SRC_DIR}/libraw/libraw_const.h"
"${libRAW_SRC_DIR}/libraw/libraw_datastream.h"
"${libRAW_SRC_DIR}/libraw/libraw_internal.h"
"${libRAW_SRC_DIR}/libraw/libraw_types.h"
"${libRAW_SRC_DIR}/libraw/libraw_version.h"
)
include_directories(${libRAW_SRC_DIR})
add_library(libRAW ${libRAW_SRC_FILES} ${libRAW_HEADER_FILES})
target_link_libraries(libRAW zlib)
target_compile_definitions(libRAW PRIVATE _LIB _CRT_SECURE_NO_WARNINGS LIBRAW_NODLL)

#add libTIFF
set(libTIFF_SRC_DIR "${freeImage_SRC_DIR}/LibTIFF4")
set(libTIFF_SRC_FILES 
"${libTIFF_SRC_DIR}/tif_aux.c"
"${libTIFF_SRC_DIR}/tif_close.c"
"${libTIFF_SRC_DIR}/tif_codec.c"
"${libTIFF_SRC_DIR}/tif_color.c"
"${libTIFF_SRC_DIR}/tif_compress.c"
"${libTIFF_SRC_DIR}/tif_dir.c"
"${libTIFF_SRC_DIR}/tif_dirinfo.c"
"${libTIFF_SRC_DIR}/tif_dirread.c"
"${libTIFF_SRC_DIR}/tif_dirwrite.c"
"${libTIFF_SRC_DIR}/tif_dumpmode.c"
"${libTIFF_SRC_DIR}/tif_error.c"
"${libTIFF_SRC_DIR}/tif_extension.c"
"${libTIFF_SRC_DIR}/tif_fax3.c"
"${libTIFF_SRC_DIR}/tif_fax3sm.c"
"${libTIFF_SRC_DIR}/tif_flush.c"
"${libTIFF_SRC_DIR}/tif_getimage.c"
"${libTIFF_SRC_DIR}/tif_jpeg.c"
"${libTIFF_SRC_DIR}/tif_luv.c"
"${libTIFF_SRC_DIR}/tif_lzma.c"
"${libTIFF_SRC_DIR}/tif_lzw.c"
"${libTIFF_SRC_DIR}/tif_next.c"
"${libTIFF_SRC_DIR}/tif_ojpeg.c"
"${libTIFF_SRC_DIR}/tif_open.c"
"${libTIFF_SRC_DIR}/tif_packbits.c"
"${libTIFF_SRC_DIR}/tif_pixarlog.c"
"${libTIFF_SRC_DIR}/tif_predict.c"
"${libTIFF_SRC_DIR}/tif_print.c"
"${libTIFF_SRC_DIR}/tif_read.c"
"${libTIFF_SRC_DIR}/tif_strip.c"
"${libTIFF_SRC_DIR}/tif_swab.c"
"${libTIFF_SRC_DIR}/tif_thunder.c"
"${libTIFF_SRC_DIR}/tif_tile.c"
"${libTIFF_SRC_DIR}/tif_version.c"
"${libTIFF_SRC_DIR}/tif_warning.c"
"${libTIFF_SRC_DIR}/tif_write.c"
"${libTIFF_SRC_DIR}/tif_zip.c"
"${libTIFF_SRC_DIR}/t4.h"
"${libTIFF_SRC_DIR}/tif_config.h"
"${libTIFF_SRC_DIR}/tif_dir.h"
"${libTIFF_SRC_DIR}/tif_fax3.h"
"${libTIFF_SRC_DIR}/tif_predict.h"
"${libTIFF_SRC_DIR}/tiff.h"
"${libTIFF_SRC_DIR}/tiffconf.h"
"${libTIFF_SRC_DIR}/tiffio.h"
"${libTIFF_SRC_DIR}/tiffiop.h"
"${libTIFF_SRC_DIR}/tiffvers.h"
"${libTIFF_SRC_DIR}/uvcode.h"
)
add_library(libTIFF ${libTIFF_SRC_FILES})
target_link_libraries(libTIFF  zlib)
target_compile_definitions(libTIFF PRIVATE _LIB _CRT_SECURE_NO_DEPRECATE)

#add libWeb
set(libWEB_SRC_DIR "${freeImage_SRC_DIR}/LibWebP")
include_directories("${libWEB_SRC_DIR}/src")
set(libWEB_SRC_FILES 
"${libWEB_SRC_DIR}/src/dec/alphai.h"
"${libWEB_SRC_DIR}/src/dec/decode_vp8.h"
"${libWEB_SRC_DIR}/src/dec/vp8i.h"
"${libWEB_SRC_DIR}/src/dec/vp8li.h"
"${libWEB_SRC_DIR}/src/dec/webpi.h"
"${libWEB_SRC_DIR}/src/dsp/dsp.h"
"${libWEB_SRC_DIR}/src/dsp/lossless.h"
"${libWEB_SRC_DIR}/src/dsp/mips_macro.h"
"${libWEB_SRC_DIR}/src/dsp/neon.h"
"${libWEB_SRC_DIR}/src/dsp/yuv.h"
"${libWEB_SRC_DIR}/src/dsp/yuv_tables_sse2.h"
"${libWEB_SRC_DIR}/src/enc/backward_references.h"
"${libWEB_SRC_DIR}/src/enc/cost.h"
"${libWEB_SRC_DIR}/src/enc/histogram.h"
"${libWEB_SRC_DIR}/src/enc/vp8enci.h"
"${libWEB_SRC_DIR}/src/enc/vp8li.h"
"${libWEB_SRC_DIR}/src/webp/decode.h"
"${libWEB_SRC_DIR}/src/webp/demux.h"
"${libWEB_SRC_DIR}/src/webp/encode.h"
"${libWEB_SRC_DIR}/src/webp/format_constants.h"
"${libWEB_SRC_DIR}/src/webp/mux.h"
"${libWEB_SRC_DIR}/src/webp/mux_types.h"
"${libWEB_SRC_DIR}/src/webp/types.h"
"${libWEB_SRC_DIR}/src/utils/bit_reader.h"
"${libWEB_SRC_DIR}/src/utils/bit_reader_inl.h"
"${libWEB_SRC_DIR}/src/utils/bit_writer.h"
"${libWEB_SRC_DIR}/src/utils/color_cache.h"
"${libWEB_SRC_DIR}/src/utils/endian_inl.h"
"${libWEB_SRC_DIR}/src/utils/filters.h"
"${libWEB_SRC_DIR}/src/utils/huffman.h"
"${libWEB_SRC_DIR}/src/utils/huffman_encode.h"
"${libWEB_SRC_DIR}/src/utils/quant_levels.h"
"${libWEB_SRC_DIR}/src/utils/quant_levels_dec.h"
"${libWEB_SRC_DIR}/src/utils/random.h"
"${libWEB_SRC_DIR}/src/utils/rescaler.h"
"${libWEB_SRC_DIR}/src/utils/thread.h"
"${libWEB_SRC_DIR}/src/utils/utils.h"
"${libWEB_SRC_DIR}/src/mux/muxi.h"
"${libWEB_SRC_DIR}/src/dec/dec.alpha.c"
"${libWEB_SRC_DIR}/src/dec/dec.buffer.c"
"${libWEB_SRC_DIR}/src/dec/dec.frame.c"
"${libWEB_SRC_DIR}/src/dec/dec.idec.c"
"${libWEB_SRC_DIR}/src/dec/dec.io.c"
"${libWEB_SRC_DIR}/src/dec/dec.quant.c"
"${libWEB_SRC_DIR}/src/dec/dec.tree.c"
"${libWEB_SRC_DIR}/src/dec/dec.vp8.c"
"${libWEB_SRC_DIR}/src/dec/dec.vp8l.c"
"${libWEB_SRC_DIR}/src/dec/dec.webp.c"
"${libWEB_SRC_DIR}/src/demux/demux.demux.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.alpha_processing.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.alpha_processing_mips_dsp_r2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.alpha_processing_sse2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.argb.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.argb_mips_dsp_r2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.argb_sse2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.cost.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.cost_mips32.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.cost_mips_dsp_r2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.cost_sse2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.cpu.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.dec.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.dec_clip_tables.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.dec_mips32.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.dec_mips_dsp_r2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.dec_neon.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.dec_sse2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.enc.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.enc_avx2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.enc_mips32.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.enc_mips_dsp_r2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.enc_neon.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.enc_sse2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.filters.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.filters_mips_dsp_r2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.filters_sse2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.lossless.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.lossless_mips32.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.lossless_mips_dsp_r2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.lossless_neon.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.lossless_sse2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.rescaler.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.rescaler_mips32.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.rescaler_mips_dsp_r2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.upsampling.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.upsampling_mips_dsp_r2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.upsampling_neon.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.upsampling_sse2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.yuv.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.yuv_mips32.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.yuv_mips_dsp_r2.c"
"${libWEB_SRC_DIR}/src/dsp/dsp.yuv_sse2.c"
"${libWEB_SRC_DIR}/src/enc/enc.alpha.c"
"${libWEB_SRC_DIR}/src/enc/enc.analysis.c"
"${libWEB_SRC_DIR}/src/enc/enc.backward_references.c"
"${libWEB_SRC_DIR}/src/enc/enc.config.c"
"${libWEB_SRC_DIR}/src/enc/enc.cost.c"
"${libWEB_SRC_DIR}/src/enc/enc.filter.c"
"${libWEB_SRC_DIR}/src/enc/enc.frame.c"
"${libWEB_SRC_DIR}/src/enc/enc.histogram.c"
"${libWEB_SRC_DIR}/src/enc/enc.iterator.c"
"${libWEB_SRC_DIR}/src/enc/enc.near_lossless.c"
"${libWEB_SRC_DIR}/src/enc/enc.picture.c"
"${libWEB_SRC_DIR}/src/enc/enc.picture_csp.c"
"${libWEB_SRC_DIR}/src/enc/enc.picture_psnr.c"
"${libWEB_SRC_DIR}/src/enc/enc.picture_rescale.c"
"${libWEB_SRC_DIR}/src/enc/enc.picture_tools.c"
"${libWEB_SRC_DIR}/src/enc/enc.quant.c"
"${libWEB_SRC_DIR}/src/enc/enc.syntax.c"
"${libWEB_SRC_DIR}/src/enc/enc.token.c"
"${libWEB_SRC_DIR}/src/enc/enc.tree.c"
"${libWEB_SRC_DIR}/src/enc/enc.vp8l.c"
"${libWEB_SRC_DIR}/src/enc/enc.webpenc.c"
"${libWEB_SRC_DIR}/src/mux/mux.anim_encode.c"
"${libWEB_SRC_DIR}/src/mux/mux.muxedit.c"
"${libWEB_SRC_DIR}/src/mux/mux.muxinternal.c"
"${libWEB_SRC_DIR}/src/mux/mux.muxread.c"
"${libWEB_SRC_DIR}/src/utils/utils.bit_reader.c"
"${libWEB_SRC_DIR}/src/utils/utils.bit_writer.c"
"${libWEB_SRC_DIR}/src/utils/utils.color_cache.c"
"${libWEB_SRC_DIR}/src/utils/utils.filters.c"
"${libWEB_SRC_DIR}/src/utils/utils.huffman.c"
"${libWEB_SRC_DIR}/src/utils/utils.huffman_encode.c"
"${libWEB_SRC_DIR}/src/utils/utils.quant_levels.c"
"${libWEB_SRC_DIR}/src/utils/utils.quant_levels_dec.c"
"${libWEB_SRC_DIR}/src/utils/utils.random.c"
"${libWEB_SRC_DIR}/src/utils/utils.rescaler.c"
"${libWEB_SRC_DIR}/src/utils/utils.thread.c"
"${libWEB_SRC_DIR}/src/utils/utils.utils.c"
)
add_library(libWeb ${libWEB_SRC_FILES})
target_link_libraries(libWeb zlib)
target_compile_definitions(libWeb PRIVATE _LIB WIN32_LEAN_AND_MEAN)

#add openEXR
set(openEXR_SRC_DIR "${freeImage_SRC_DIR}/OpenEXR")
include_directories(
${openEXR_SRC_DIR}
"${openEXR_SRC_DIR}/IlmImf"
"${openEXR_SRC_DIR}/Imath"
"${openEXR_SRC_DIR}/IexMath"
"${openEXR_SRC_DIR}/Iex"
"${openEXR_SRC_DIR}/Half"
"${openEXR_SRC_DIR}/IlmThread"
)
set(openEXR_SRC_FILES 
"${openEXR_SRC_DIR}/IexMath/IexMathFpu.cpp"
"${openEXR_SRC_DIR}/IlmImf/b44ExpLogTable.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfAcesFile.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfB44Compressor.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfBoxAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfChannelList.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfChannelListAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfChromaticities.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfChromaticitiesAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfCompositeDeepScanLine.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfCompressionAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfCompressor.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfConvert.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfCRgbaFile.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepCompositing.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepFrameBuffer.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepImageStateAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepScanLineInputFile.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepScanLineInputPart.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepScanLineOutputFile.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepScanLineOutputPart.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepTiledInputFile.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepTiledInputPart.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepTiledOutputFile.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepTiledOutputPart.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfDoubleAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfDwaCompressor.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfEnvmap.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfEnvmapAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfFastHuf.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfFloatAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfFloatVectorAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfFrameBuffer.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfFramesPerSecond.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfGenericInputFile.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfGenericOutputFile.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfHeader.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfHuf.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfInputFile.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfInputPart.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfInputPartData.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfIntAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfIO.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfKeyCode.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfKeyCodeAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfLineOrderAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfLut.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfMatrixAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfMisc.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfMultiPartInputFile.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfMultiPartOutputFile.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfMultiView.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfOpaqueAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfOutputFile.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfOutputPart.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfOutputPartData.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfPartType.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfPizCompressor.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfPreviewImage.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfPreviewImageAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfPxr24Compressor.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfRational.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfRationalAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfRgbaFile.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfRgbaYca.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfRle.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfRleCompressor.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfScanLineInputFile.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfStandardAttributes.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfStdIO.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfStringAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfStringVectorAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfSystemSpecific.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfTestFile.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfThreading.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfTileDescriptionAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfTiledInputFile.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfTiledInputPart.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfTiledMisc.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfTiledOutputFile.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfTiledOutputPart.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfTiledRgbaFile.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfTileOffsets.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfTimeCode.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfTimeCodeAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfVecAttribute.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfVersion.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfWav.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfZip.cpp"
"${openEXR_SRC_DIR}/IlmImf/ImfZipCompressor.cpp"
"${openEXR_SRC_DIR}/Imath/ImathBox.cpp"
"${openEXR_SRC_DIR}/Imath/ImathColorAlgo.cpp"
"${openEXR_SRC_DIR}/Imath/ImathFun.cpp"
"${openEXR_SRC_DIR}/Imath/ImathMatrixAlgo.cpp"
"${openEXR_SRC_DIR}/Imath/ImathRandom.cpp"
"${openEXR_SRC_DIR}/Imath/ImathShear.cpp"
"${openEXR_SRC_DIR}/Imath/ImathVec.cpp"
"${openEXR_SRC_DIR}/Iex/IexBaseExc.cpp"
"${openEXR_SRC_DIR}/Iex/IexThrowErrnoExc.cpp"
"${openEXR_SRC_DIR}/Half/half.cpp"
"${openEXR_SRC_DIR}/IlmThread/IlmThread.cpp"
"${openEXR_SRC_DIR}/IlmThread/IlmThreadMutex.cpp"
"${openEXR_SRC_DIR}/IlmThread/IlmThreadPool.cpp"
"${openEXR_SRC_DIR}/IlmThread/IlmThreadSemaphore.cpp"
"${openEXR_SRC_DIR}/IexMath/IexMathFloatExc.cpp"
"${openEXR_SRC_DIR}/IlmBaseConfig.h"
"${openEXR_SRC_DIR}/OpenEXRConfig.h"
"${openEXR_SRC_DIR}/IlmImf/b44ExpLogTable.h"
"${openEXR_SRC_DIR}/IlmImf/dwaLookups.h"
"${openEXR_SRC_DIR}/IlmImf/ImfAcesFile.h"
"${openEXR_SRC_DIR}/IlmImf/ImfArray.h"
"${openEXR_SRC_DIR}/IlmImf/ImfAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfAutoArray.h"
"${openEXR_SRC_DIR}/IlmImf/ImfB44Compressor.h"
"${openEXR_SRC_DIR}/IlmImf/ImfBoxAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfChannelList.h"
"${openEXR_SRC_DIR}/IlmImf/ImfChannelListAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfCheckedArithmetic.h"
"${openEXR_SRC_DIR}/IlmImf/ImfChromaticities.h"
"${openEXR_SRC_DIR}/IlmImf/ImfChromaticitiesAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfCompositeDeepScanLine.h"
"${openEXR_SRC_DIR}/IlmImf/ImfCompression.h"
"${openEXR_SRC_DIR}/IlmImf/ImfCompressionAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfCompressor.h"
"${openEXR_SRC_DIR}/IlmImf/ImfConvert.h"
"${openEXR_SRC_DIR}/IlmImf/ImfCRgbaFile.h"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepCompositing.h"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepFrameBuffer.h"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepImageState.h"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepImageStateAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepScanLineInputFile.h"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepScanLineInputPart.h"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepScanLineOutputFile.h"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepScanLineOutputPart.h"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepTiledInputFile.h"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepTiledInputPart.h"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepTiledOutputFile.h"
"${openEXR_SRC_DIR}/IlmImf/ImfDeepTiledOutputPart.h"
"${openEXR_SRC_DIR}/IlmImf/ImfDoubleAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfDwaCompressor.h"
"${openEXR_SRC_DIR}/IlmImf/ImfDwaCompressorSimd.h"
"${openEXR_SRC_DIR}/IlmImf/ImfEnvmap.h"
"${openEXR_SRC_DIR}/IlmImf/ImfEnvmapAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfExport.h"
"${openEXR_SRC_DIR}/IlmImf/ImfFastHuf.h"
"${openEXR_SRC_DIR}/IlmImf/ImfFloatAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfFloatVectorAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfForward.h"
"${openEXR_SRC_DIR}/IlmImf/ImfFrameBuffer.h"
"${openEXR_SRC_DIR}/IlmImf/ImfFramesPerSecond.h"
"${openEXR_SRC_DIR}/IlmImf/ImfGenericInputFile.h"
"${openEXR_SRC_DIR}/IlmImf/ImfGenericOutputFile.h"
"${openEXR_SRC_DIR}/IlmImf/ImfHeader.h"
"${openEXR_SRC_DIR}/IlmImf/ImfHuf.h"
"${openEXR_SRC_DIR}/IlmImf/ImfInputFile.h"
"${openEXR_SRC_DIR}/IlmImf/ImfInputPart.h"
"${openEXR_SRC_DIR}/IlmImf/ImfInputPartData.h"
"${openEXR_SRC_DIR}/IlmImf/ImfInputStreamMutex.h"
"${openEXR_SRC_DIR}/IlmImf/ImfInt64.h"
"${openEXR_SRC_DIR}/IlmImf/ImfIntAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfIO.h"
"${openEXR_SRC_DIR}/IlmImf/ImfKeyCode.h"
"${openEXR_SRC_DIR}/IlmImf/ImfKeyCodeAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfLineOrder.h"
"${openEXR_SRC_DIR}/IlmImf/ImfLineOrderAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfLut.h"
"${openEXR_SRC_DIR}/IlmImf/ImfMatrixAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfMisc.h"
"${openEXR_SRC_DIR}/IlmImf/ImfMultiPartInputFile.h"
"${openEXR_SRC_DIR}/IlmImf/ImfMultiPartOutputFile.h"
"${openEXR_SRC_DIR}/IlmImf/ImfMultiView.h"
"${openEXR_SRC_DIR}/IlmImf/ImfName.h"
"${openEXR_SRC_DIR}/IlmImf/ImfNamespace.h"
"${openEXR_SRC_DIR}/IlmImf/ImfOpaqueAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfOptimizedPixelReading.h"
"${openEXR_SRC_DIR}/IlmImf/ImfOutputFile.h"
"${openEXR_SRC_DIR}/IlmImf/ImfOutputPart.h"
"${openEXR_SRC_DIR}/IlmImf/ImfOutputPartData.h"
"${openEXR_SRC_DIR}/IlmImf/ImfOutputStreamMutex.h"
"${openEXR_SRC_DIR}/IlmImf/ImfPartHelper.h"
"${openEXR_SRC_DIR}/IlmImf/ImfPartType.h"
"${openEXR_SRC_DIR}/IlmImf/ImfPixelType.h"
"${openEXR_SRC_DIR}/IlmImf/ImfPizCompressor.h"
"${openEXR_SRC_DIR}/IlmImf/ImfPreviewImage.h"
"${openEXR_SRC_DIR}/IlmImf/ImfPreviewImageAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfPxr24Compressor.h"
"${openEXR_SRC_DIR}/IlmImf/ImfRational.h"
"${openEXR_SRC_DIR}/IlmImf/ImfRationalAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfRgba.h"
"${openEXR_SRC_DIR}/IlmImf/ImfRgbaFile.h"
"${openEXR_SRC_DIR}/IlmImf/ImfRgbaYca.h"
"${openEXR_SRC_DIR}/IlmImf/ImfRle.h"
"${openEXR_SRC_DIR}/IlmImf/ImfRleCompressor.h"
"${openEXR_SRC_DIR}/IlmImf/ImfScanLineInputFile.h"
"${openEXR_SRC_DIR}/IlmImf/ImfSimd.h"
"${openEXR_SRC_DIR}/IlmImf/ImfStandardAttributes.h"
"${openEXR_SRC_DIR}/IlmImf/ImfStdIO.h"
"${openEXR_SRC_DIR}/IlmImf/ImfStringAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfStringVectorAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfSystemSpecific.h"
"${openEXR_SRC_DIR}/IlmImf/ImfTestFile.h"
"${openEXR_SRC_DIR}/IlmImf/ImfThreading.h"
"${openEXR_SRC_DIR}/IlmImf/ImfTileDescription.h"
"${openEXR_SRC_DIR}/IlmImf/ImfTileDescriptionAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfTiledInputFile.h"
"${openEXR_SRC_DIR}/IlmImf/ImfTiledInputPart.h"
"${openEXR_SRC_DIR}/IlmImf/ImfTiledMisc.h"
"${openEXR_SRC_DIR}/IlmImf/ImfTiledOutputFile.h"
"${openEXR_SRC_DIR}/IlmImf/ImfTiledOutputPart.h"
"${openEXR_SRC_DIR}/IlmImf/ImfTiledRgbaFile.h"
"${openEXR_SRC_DIR}/IlmImf/ImfTileOffsets.h"
"${openEXR_SRC_DIR}/IlmImf/ImfTimeCode.h"
"${openEXR_SRC_DIR}/IlmImf/ImfTimeCodeAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfVecAttribute.h"
"${openEXR_SRC_DIR}/IlmImf/ImfVersion.h"
"${openEXR_SRC_DIR}/IlmImf/ImfWav.h"
"${openEXR_SRC_DIR}/IlmImf/ImfXdr.h"
"${openEXR_SRC_DIR}/IlmImf/ImfZip.h"
"${openEXR_SRC_DIR}/IlmImf/ImfZipCompressor.h"
"${openEXR_SRC_DIR}/Imath/ImathBox.h"
"${openEXR_SRC_DIR}/Imath/ImathBoxAlgo.h"
"${openEXR_SRC_DIR}/Imath/ImathColor.h"
"${openEXR_SRC_DIR}/Imath/ImathColorAlgo.h"
"${openEXR_SRC_DIR}/Imath/ImathEuler.h"
"${openEXR_SRC_DIR}/Imath/ImathExc.h"
"${openEXR_SRC_DIR}/Imath/ImathExport.h"
"${openEXR_SRC_DIR}/Imath/ImathForward.h"
"${openEXR_SRC_DIR}/Imath/ImathFrame.h"
"${openEXR_SRC_DIR}/Imath/ImathFrustum.h"
"${openEXR_SRC_DIR}/Imath/ImathFrustumTest.h"
"${openEXR_SRC_DIR}/Imath/ImathFun.h"
"${openEXR_SRC_DIR}/Imath/ImathGL.h"
"${openEXR_SRC_DIR}/Imath/ImathGLU.h"
"${openEXR_SRC_DIR}/Imath/ImathHalfLimits.h"
"${openEXR_SRC_DIR}/Imath/ImathInt64.h"
"${openEXR_SRC_DIR}/Imath/ImathInterval.h"
"${openEXR_SRC_DIR}/Imath/ImathLimits.h"
"${openEXR_SRC_DIR}/Imath/ImathLine.h"
"${openEXR_SRC_DIR}/Imath/ImathLineAlgo.h"
"${openEXR_SRC_DIR}/Imath/ImathMath.h"
"${openEXR_SRC_DIR}/Imath/ImathMatrix.h"
"${openEXR_SRC_DIR}/Imath/ImathMatrixAlgo.h"
"${openEXR_SRC_DIR}/Imath/ImathNamespace.h"
"${openEXR_SRC_DIR}/Imath/ImathPlane.h"
"${openEXR_SRC_DIR}/Imath/ImathPlatform.h"
"${openEXR_SRC_DIR}/Imath/ImathQuat.h"
"${openEXR_SRC_DIR}/Imath/ImathRandom.h"
"${openEXR_SRC_DIR}/Imath/ImathRoots.h"
"${openEXR_SRC_DIR}/Imath/ImathShear.h"
"${openEXR_SRC_DIR}/Imath/ImathSphere.h"
"${openEXR_SRC_DIR}/Imath/ImathVec.h"
"${openEXR_SRC_DIR}/Imath/ImathVecAlgo.h"
"${openEXR_SRC_DIR}/Iex/Iex.h"
"${openEXR_SRC_DIR}/Iex/IexBaseExc.h"
"${openEXR_SRC_DIR}/Iex/IexErrnoExc.h"
"${openEXR_SRC_DIR}/Iex/IexExport.h"
"${openEXR_SRC_DIR}/Iex/IexForward.h"
"${openEXR_SRC_DIR}/Iex/IexMacros.h"
"${openEXR_SRC_DIR}/Iex/IexMathExc.h"
"${openEXR_SRC_DIR}/Iex/IexNamespace.h"
"${openEXR_SRC_DIR}/Iex/IexThrowErrnoExc.h"
"${openEXR_SRC_DIR}/Half/eLut.h"
"${openEXR_SRC_DIR}/Half/half.h"
"${openEXR_SRC_DIR}/Half/halfFunction.h"
"${openEXR_SRC_DIR}/Half/halfLimits.h"
"${openEXR_SRC_DIR}/Half/toFloat.h"
"${openEXR_SRC_DIR}/IlmThread/IlmThread.h"
"${openEXR_SRC_DIR}/IlmThread/IlmThreadExport.h"
"${openEXR_SRC_DIR}/IlmThread/IlmThreadForward.h"
"${openEXR_SRC_DIR}/IlmThread/IlmThreadMutex.h"
"${openEXR_SRC_DIR}/IlmThread/IlmThreadNamespace.h"
"${openEXR_SRC_DIR}/IlmThread/IlmThreadPool.h"
"${openEXR_SRC_DIR}/IlmThread/IlmThreadSemaphore.h"
"${openEXR_SRC_DIR}/IexMath/IexMathFloatExc.h"
"${openEXR_SRC_DIR}/IexMath/IexMathIeeeExc.h"
)
add_library(openEXR ${openEXR_SRC_FILES})
target_link_libraries(openEXR zlib)
target_compile_definitions(openEXR PRIVATE _LIB _CRT_SECURE_NO_DEPRECATE)

#add freeImageLib
set(freeImageLib_SRC_DIR "${freeImage_SRC_DIR}/FreeImageLib")
set(freeImageLib_SRC_FILES 
"${freeImage_SRC_DIR}/FreeImage/BitmapAccess.cpp"
"${freeImage_SRC_DIR}/FreeImage/ColorLookup.cpp"
"${freeImage_SRC_DIR}/FreeImage/ConversionRGBA16.cpp"
"${freeImage_SRC_DIR}/FreeImage/ConversionRGBAF.cpp"
"${freeImage_SRC_DIR}/FreeImage/FreeImage.cpp"
"${freeImage_SRC_DIR}/FreeImage/FreeImageIO.cpp"
"${freeImage_SRC_DIR}/FreeImage/GetType.cpp"
"${freeImage_SRC_DIR}/FreeImage/LFPQuantizer.cpp"
"${freeImage_SRC_DIR}/FreeImage/MemoryIO.cpp"
"${freeImage_SRC_DIR}/FreeImage/PixelAccess.cpp"
"${freeImage_SRC_DIR}/FreeImage/NNQuantizer.cpp"
"${freeImage_SRC_DIR}/FreeImage/WuQuantizer.cpp"
"${freeImage_SRC_DIR}/FreeImage/Conversion.cpp"
"${freeImage_SRC_DIR}/FreeImage/Conversion16_555.cpp"
"${freeImage_SRC_DIR}/FreeImage/Conversion16_565.cpp"
"${freeImage_SRC_DIR}/FreeImage/Conversion24.cpp"
"${freeImage_SRC_DIR}/FreeImage/Conversion32.cpp"
"${freeImage_SRC_DIR}/FreeImage/Conversion4.cpp"
"${freeImage_SRC_DIR}/FreeImage/Conversion8.cpp"
"${freeImage_SRC_DIR}/FreeImage/ConversionFloat.cpp"
"${freeImage_SRC_DIR}/FreeImage/ConversionRGB16.cpp"
"${freeImage_SRC_DIR}/FreeImage/ConversionRGBF.cpp"
"${freeImage_SRC_DIR}/FreeImage/ConversionType.cpp"
"${freeImage_SRC_DIR}/FreeImage/ConversionUINT16.cpp"
"${freeImage_SRC_DIR}/FreeImage/Halftoning.cpp"
"${freeImage_SRC_DIR}/FreeImage/tmoColorConvert.cpp"
"${freeImage_SRC_DIR}/FreeImage/tmoDrago03.cpp"
"${freeImage_SRC_DIR}/FreeImage/tmoFattal02.cpp"
"${freeImage_SRC_DIR}/FreeImage/tmoReinhard05.cpp"
"${freeImage_SRC_DIR}/FreeImage/ToneMapping.cpp"
"${freeImage_SRC_DIR}/FreeImage/J2KHelper.cpp"
"${freeImage_SRC_DIR}/FreeImage/MNGHelper.cpp"
"${freeImage_SRC_DIR}/FreeImage/Plugin.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginBMP.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginCUT.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginDDS.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginEXR.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginG3.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginGIF.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginHDR.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginICO.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginIFF.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginJ2K.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginJNG.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginJP2.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginJPEG.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginJXR.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginKOALA.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginMNG.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginPCD.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginPCX.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginPFM.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginPICT.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginPNG.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginPNM.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginPSD.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginRAS.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginRAW.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginSGI.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginTARGA.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginTIFF.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginWBMP.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginWebP.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginXBM.cpp"
"${freeImage_SRC_DIR}/FreeImage/PluginXPM.cpp"
"${freeImage_SRC_DIR}/FreeImage/PSDParser.cpp"
"${freeImage_SRC_DIR}/FreeImage/TIFFLogLuv.cpp"
"${freeImage_SRC_DIR}/DeprecationManager/Deprecated.cpp"
"${freeImage_SRC_DIR}/DeprecationManager/DeprecationMgr.cpp"
"${freeImage_SRC_DIR}/FreeImage/CacheFile.cpp"
"${freeImage_SRC_DIR}/FreeImage/MultiPage.cpp"
"${freeImage_SRC_DIR}/FreeImage/ZLibInterface.cpp"
"${freeImage_SRC_DIR}/Metadata/Exif.cpp"
"${freeImage_SRC_DIR}/Metadata/FIRational.cpp"
"${freeImage_SRC_DIR}/Metadata/FreeImageTag.cpp"
"${freeImage_SRC_DIR}/Metadata/IPTC.cpp"
"${freeImage_SRC_DIR}/Metadata/TagConversion.cpp"
"${freeImage_SRC_DIR}/Metadata/TagLib.cpp"
"${freeImage_SRC_DIR}/Metadata/XTIFF.cpp"
"${freeImage_SRC_DIR}/FreeImageToolkit/Background.cpp"
"${freeImage_SRC_DIR}/FreeImageToolkit/BSplineRotate.cpp"
"${freeImage_SRC_DIR}/FreeImageToolkit/Channels.cpp"
"${freeImage_SRC_DIR}/FreeImageToolkit/ClassicRotate.cpp"
"${freeImage_SRC_DIR}/FreeImageToolkit/Colors.cpp"
"${freeImage_SRC_DIR}/FreeImageToolkit/CopyPaste.cpp"
"${freeImage_SRC_DIR}/FreeImageToolkit/Display.cpp"
"${freeImage_SRC_DIR}/FreeImageToolkit/Flip.cpp"
"${freeImage_SRC_DIR}/FreeImageToolkit/JPEGTransform.cpp"
"${freeImage_SRC_DIR}/FreeImageToolkit/MultigridPoissonSolver.cpp"
"${freeImage_SRC_DIR}/FreeImageToolkit/Rescale.cpp"
"${freeImage_SRC_DIR}/FreeImageToolkit/Resize.cpp"
"${freeImage_SRC_DIR}/CacheFile.h"
"${freeImage_SRC_DIR}/DeprecationManager/DeprecationMgr.h"
"${freeImage_SRC_DIR}/MapIntrospector.h"
"${freeImage_SRC_DIR}/Metadata/FIRational.h"
"${freeImage_SRC_DIR}/FreeImage.h"
"${freeImage_SRC_DIR}/FreeImageIO.h"
"${freeImage_SRC_DIR}/Metadata/FreeImageTag.h"
"${freeImage_SRC_DIR}/Plugin.h"
"${freeImage_SRC_DIR}/FreeImage/PSDParser.h"
"${freeImage_SRC_DIR}/Quantizers.h"
"${freeImage_SRC_DIR}/ToneMapping.h"
"${freeImage_SRC_DIR}/Utilities.h"
"${freeImage_SRC_DIR}/FreeImageToolkit/Resize.h"
)
include_directories(
"${freeImage_SRC_DIR}"
"${freeImage_SRC_DIR}/ZLib"
"${freeImage_SRC_DIR}/DeprecationManager"
"${freeImage_SRC_DIR}/OpenEXR"
"${freeImage_SRC_DIR}/OpenEXR/Half"
"${freeImage_SRC_DIR}/OpenEXR/Iex"
"${freeImage_SRC_DIR}/OpenEXR/IlmImf"
"${freeImage_SRC_DIR}/OpenEXR/Imath"
"${freeImage_SRC_DIR}/OpenEXR/IlmThread"
"${freeImage_SRC_DIR}/LibJXR/jxrgluelib"
"${freeImage_SRC_DIR}/LibJXR/image/sys"
)
add_library(freeImageLib ${freeImageLib_SRC_FILES})
target_link_libraries(freeImageLib zlib openEXR libWeb libTIFF libRAW libPNG libJXR libOpenJPEG libJPEG)
target_compile_definitions(freeImageLib PRIVATE _USRDLL OPJ_STATIC FREEIMAGE_EXPORTS FREEIMAGE_LIB _CRT_SECURE_NO_DEPRECATE LIBRAW_NODLL)

#create vs folder structure
set_target_properties(freeImageLib openEXR libWeb libTIFF libRAW libPNG libJXR libOpenJPEG libJPEG zlib glew glew_s glewmx glewmx_s glfw PROPERTIES FOLDER "external")