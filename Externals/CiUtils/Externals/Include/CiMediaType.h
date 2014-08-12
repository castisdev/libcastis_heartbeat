#pragma once

#include <string>

namespace castis {

	namespace mediaType
	{
		enum SystemType
		{
			SYSTEM_UNKNOWN =		0x00000000,				// unknown system type

			SYSTEM_MPEG1_SYSTEM =	0x00010000,
			SYSTEM_MPEG2_TS =		0x00020000,
			SYSTEM_MPEG2_PS =		0x00030000,
			SYSTEM_MP4 =			0x00040000,
			SYSTEM_AVI =			0x00050000,
			SYSTEM_GIF =			0x00060000,
			SYSTEM_JPG =			0x00070000,
			SYSTEM_MP3 =			0x00080000,
			SYSTEM_DAT =			0x00090000,
			SYSTEM_SVCD =			0x000A0000,
			SYSTEM_VOB =			0x000B0000,
			SYSTEM_ASF =			0x000C0000,

			SYSTEM_NONE	=			0x00FF0000				// no system in the media.. not used
		};

		inline std::string getSystemTypeString(SystemType mtSystem)
		{
			switch (mtSystem)
			{
			case SYSTEM_MPEG1_SYSTEM :
				return "MPEG1";

			case SYSTEM_MPEG2_TS :
				return "MPEG2-TS";

			case SYSTEM_MPEG2_PS :
				return "MPEG2-PS";

			case SYSTEM_MP4 :
				return "MP4";

			case SYSTEM_AVI :
				return "AVI";

			case SYSTEM_GIF :
				return "GIF";

			case SYSTEM_JPG :
				return "JPG";

			case SYSTEM_MP3 :
				return "MP3";

			case SYSTEM_DAT :
				return "DAT";

			case SYSTEM_SVCD :
				return "SVCD";

			case SYSTEM_VOB :
				return "VOB";

			case SYSTEM_ASF :
				return "ASF";

			case SYSTEM_NONE :
				return "None";

			default:
				return "Unknown";
			}
		};

		////////////////////////////////////////////////////////
		/// element type
		////////////////////////////////////////////////////////
		enum ElementType
		{
			eElementUnknown = 0x00000000

			/// video type.
			,eVideoUnknown = 0x00000000
			,eVideoMPEG1 = 0x00000100	///< ISO 11172-2
			,eVideoMPEG2 = 0x00000200	///< ISO 13818-2
			,eVideoMPEG4 = 0x00000300	///< ISO 14496-2
			,eVideoMPEG4AVC = 0x00000400	///< ISO 14496-10/ ITU-T H.264
			,eVideoVC1		= 0x00000500	///< SMPTE 421m VC1( Microsoft WMV3 )
			,eVideoHEVC		= 0x00000600	///< ISO 23008-2
			
			,eVideoNone = 0x0000FF00

			/// audio type.
			,eAudioUnknown = 0x00000000
			,eAudioMPEG1 = 0x00000001	///< ISO 11172-3(13818-3 compatible)
			,eAudioMPEG2 = 0x00000002	///< ISO 13818-3
			,eAudioAC3 = 0x00000004		///< ATSC AC3( Dolby Digital)
			,eAudioADS = 0x00000005		///< sony PS2 ADS
			,eAudioMPEGAAC = 0x00000006 ///< ISO 13818-7/14496-3. 기존의 MPEG2_AAC와 value 호환
			//eAudioDTS = 0x00000008	///< Digital Theater System.  is AUDIO_DTS and undefined and unused
			//AUDIO_AAC (3), AUDIO_MPEG2_AAC(6), AUDIO_MPEG4_AAC(7), AUDIO_MPEG4(9) : from 0x00000003 ~ to 0x00000009 used for AAC
			,eAudioWMA2	= 0x0000000A	///< Windows Media Audio 7,8,9 series (v2)
		
			,eAudioNone = 0x000000FF

		};

		enum
		{
			eVideoKnown = (eVideoMPEG1|eVideoMPEG2|eVideoMPEG4|eVideoMPEG4AVC|eVideoVC1)
			,eAudioKnown = (eAudioMPEG1|eAudioMPEG2|eAudioAC3|eAudioADS|eAudioMPEGAAC)
		};

		/// only for known type.
		inline bool isAudio(ElementType mt)
		{
			if ( mt&eAudioKnown )
				return true;
			return false;
		};

		inline bool isVideo(ElementType mt)
		{
			if ( mt&eVideoKnown)
				return true;
			return false;
		};

		inline std::string getElementTypeString(ElementType mtElement)
		{
			switch (mtElement)
			{
			case eVideoMPEG1 :
				return "MPEG1 Video";

			case eVideoMPEG2 :
				return "MPEG2 Video";

			case eVideoMPEG4 :
				return "MPEG4 Video";

			case eVideoMPEG4AVC :
				return "MPEG4-AVC(H.264)";

			case eVideoVC1:
				return "VC1";

			case eVideoHEVC:
				return "HEVC(H.265)";

			case eVideoNone :
				return "None";

			case eAudioMPEG1 :
				return "MPEG Audio";

			case eAudioMPEG2 :
				return "MPEG Audio";

			case eAudioAC3 :
				return "ATSC AC3";

			case eAudioMPEGAAC :
				return "AAC";

			case eAudioWMA2:
				return "WMA";

			case eAudioNone :
				return "None";

			default:
				return "Unknown";
			}
		};
		
	} // e.o.MediaType

	namespace media_info
	{
		enum { N_TS_BYTES = 188 };
	}

} // e.o.castis
