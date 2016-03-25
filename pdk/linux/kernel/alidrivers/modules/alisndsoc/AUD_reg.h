AUD_UINT8 AUDREG_GetVolume(void);
void ResetAudCore(void);
void StartSPODMA(AUD_BOOL Enable);
void StartI2SODMA(AUD_BOOL Enable);
void AUDREG_SetVolume(AUD_UINT8 Volume);
void SetI2SODMADataWithHeader(AUD_BOOL Enable);
void EnableI2SOInterface(AUD_BOOL Enable);
void EnableSPOInterface(AUD_BOOL Enable);
void AUDREG_ConfigInterface(AUD_SubBlock_t SubBlockIdx,
                                   AUD_OutputParams_t *Params_p);
void AUDREG_ConfigDMA(AUD_SubBlock_t SubBlockIdx,
                             AUD_UINT32 DMABase,
                             AUD_UINT16 DMALen);
AUD_BOOL GetI2SODMAStatus(void);
AUD_BOOL GetSPODMAStatus(void);
void EnableI2SIRXInterface(AUD_BOOL Enable);
void AUDREG_ClearAllAudioInterrupt(void);
void AUDREG_DisableAllAudioInterrupt(void);
AUD_UINT16 AUDREG_GetDMACurrentIndex(AUD_SubBlock_t SubBlockIdx);
AUD_UINT16 AUDREG_GetDMALastIndex(AUD_SubBlock_t SubBlockIdx);
AUD_UINT32 GetDMABufUnderRunIntStatus(void);
AUD_UINT32 GetInterfaceEnableStatus(void);
AUD_BOOL CheckI2SODMABufUnderRunIntStatus(void);
void StartI2SIRXDMA(AUD_BOOL Enable);
void AUDREG_SetDMALastIndex(AUD_SubBlock_t SubBlockIdx,
                                   AUD_UINT16 Index);
AUD_BOOL CheckI2SOSampCountInt(void);
AUD_BOOL CheckI2SOSampCountIntStatus(void);
AUD_BOOL CheckSPOSampCountInt(void);
AUD_BOOL CheckSPOSampCountIntStatus(void);
AUD_BOOL CheckSPODMABufUnderRunIntStatus(void);
void EnableI2SODMAUnderRunInt(AUD_BOOL Enable);
void EnableSPODMAUnderRunInt(AUD_BOOL Enable);
void EnableI2SOSampCountInt(AUD_BOOL Enable);
void EnableSPOSampCountInt(AUD_BOOL Enable);
void AudioPinmuxConfigurate(void);
void PrintAudioConfiguratation(void);
void AudioConfiguratationPatch(void);