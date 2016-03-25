1, it is the desctiption of the plugin of audio engine for s3921 Series Chip platform

2, all the plugin is at: ddk/ae_plugin

3, free plugins:
		
4, plugins need to be licenced (Only PM can release the plugin):
	
	/* audio decoder */
	ae_plugin_aac.o  : AAC decoder

	/* MUST and ONLY select on from below two plugins for Dolby Decoder */
  ae_plugin_ac3.o  : AC3 decoder (downmix to 2 channel)
  ae_plugin_eac3.o : ec3 decoder (downmix to 2 channel,no longer compatible 
		                              with ac3 decoder,if you want to support ac3/eac3 at the sametime,
		                              please put ae_plugin_ac3.o and ae_plugin_eac3.o together in Plugin Dir)

5, plugins don't be used in the current solution

	