#define VID_MODE_576i50 0
#define VID_MODE_480i60 1
#define VID_MODE_576p50 2
#define VID_MODE_480p60 3

void InitCAT6611();
void HDMI_Select_VideoMode(BYTE VideoMode);
void HDMI_CAT6611_check();
