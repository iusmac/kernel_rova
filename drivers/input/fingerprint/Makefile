#
# Makefile for the fingerprint drivers.
#
#
obj-$(CONFIG_FINGERPRINT_GF3208_UGG) += gf3208_ugg/
obj-$(CONFIG_FINGERPRINT_FPC1020_UGG) += fpc_ugg/

obj-$(CONFIG_FINGERPRINT_GOODIX_GF3208_LANDTONI) += goodix_landtoni/
obj-$(CONFIG_FINGERPRINT_FPC1020_LANDTONI) += fpc_landtoni/

subdir-ccflags-y := -finline-functions -ftree-vectorize -funroll-loops
