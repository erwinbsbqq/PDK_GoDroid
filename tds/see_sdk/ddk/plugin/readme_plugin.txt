1, it is the desctiption of the plugin for M3701C Series Chip platform

2, all the plugin is at: ddk/plugin

3, free plugins:
	/* common plugin for media player */
	lib_con_common.o : MUST be released for media player solution

	/* audio decoder */
	lib_ogg.o lib_con_ogg.o lib_con_vorbis.o : OGG decoder
	lib_con_pcm.o : PCM decoder
	lib_con_adpcm.o : ADPCM decoder
	lib_con_ape.o libapedec.o : APE decoder
	
	/* video decoder */
	lib_vc1dec.o lib_con_vc1.o : VC1 decoder
	lib_vp8dec.o lib_con_vp8.o : VP8 decoder		
	lib_mjpgdec.o lib_con_mjpg.o : MJPEG decoder
	lib_con_mpeg2.o : MPEG1/MPEG2 decoder
	lib_con_h264.o : H.264 decoder	
	lib_con_dts.o : DTS decoder(only support BYPASS)

	
4, plugins need to be licenced (Only PM can release the plugin):
	
	/* audio decoder */

	/* MUST and ONLY select on from below two plugins for Dolby Decoder */
	plugin_ac32c.o : AC3 decoder (downmix to 2 channel)
		plugin_eac3.o : ec3 decoder (downmix to 2 channel,no longer compatible 
		with ac3 decoder,if you want to support ac3/eac3 at the sametime,please 
		put plugin_ac32c.o and plugin_eac3.o together in Plugin Dir)
		
			
	libwmaprodec.o lib_con_wmapro.o : WMA Pro decoder
	libwmadec.o lib_con_wma.o : WMA decoder
	libmp3dec.o lib_con_mp3.o lib_mp3.o : MP3 decoder
	libflacdec.o lib_con_flac.o lib_flac.o : Flac decoder
	libalacdec.o lib_con_alac.o : Alac decoder
	lib_con_ra.o : RA decoder
	plugin_aacv2.o lib_con_aac.o : AAC decoder	
	plugin_aac.o.bak : old AAC decoder(don't be used)
	lib_con_ac3.o : AC3 decoder(only support BYPASS mode if no plugin_ac3
				or plugin_eac3 is inclued)
	lib_con_amr.o : AMR decoder
	
	/*video decoder */
	lib_rvdec.o lib_con_rv.o : RMVB decoder
	lib_mp4dec.o lib_con_mpeg4.o : MPEG4/XVID decoder(need licence for DVIX)

5, plugins don't be used in the current solution

	/* demuxer */
	lib_ts.o : TS demuxer(don't be used)
	lib_mpg.o : PS demuxer(don't be used)
	lib_mp4.o : MP4 demuxer(don't be used)
	lib_mkv.o : MKV demuxer(don't be used)
	lib_avi.o : AVI demuxer(don't be used)							
	lib_avi_subt.o : special plugin for avi(don't be used)	
	lib_avi_XD311.o : special plugin for avi(don't be used)		
	lib_wav.o : WAV demuxer(don't be used)
	
	/* subtitle */
	lib_con_subtitle.o : Subtitle for media player(don't be used)			
