file(GLOB_RECURSE etcSources ${ETC_DIR}/EtcLib/EtcCodec/*.cpp ${ETC_DIR}/EtcLib/EtcCodec/*.h)
set(etcSources ${etcSources}
	${ETC_DIR}/EtcLib/Etc/EtcImage.cpp
	${ETC_DIR}/EtcLib/Etc/EtcImage.h
	${ETC_DIR}/EtcLib/Etc/EtcMath.cpp
	${ETC_DIR}/EtcLib/Etc/EtcMath.h
	${ETC_DIR}/EtcLib/Etc/EtcConfig.h
)

set(ETC_INCLUDE_DIRS ${ETC_DIR}/EtcLib/Etc ${ETC_DIR}/EtcLib/EtcCodec)
