#include "dib8000_priv.h"

void dib8000_set_power_mode(struct dib8000_state *state, enum dib8000_power_mode mode)
{
	/* by default everything is going to be powered off */
	uint16_t reg_774 = 0x3fff, reg_775 = 0xffff, reg_776 = 0xffff,
		reg_900  = (dib8000_read_word(state, 900)  & 0xfffc) | 0x3,
		reg_1280 = (dib8000_read_word(state, 1280) & 0x00ff) | 0xff00;

	/* now, depending on the requested mode, we power on */
	switch (mode) {
		/* power up everything in the demod */
		case DIB8000M_POWER_ALL:
            reg_774 = 0x0000; reg_775 = 0x0000; reg_776 = 0x0000; reg_900 &= 0xfffc;
			reg_1280 &= 0x00ff;
			break;
		case DIB8000M_POWER_INTERFACE_ONLY:
            reg_1280 &= 0x00ff;
			break;
    }

    dbgpl(&dib8000_dbg, "powermode : 774 : %x ; 775 : %x; 776 : %x ; 900 : %x; 1280 : %x", reg_774, reg_775, reg_776, reg_900, reg_1280);
	dib8000_write_word(state,  774, reg_774);
	dib8000_write_word(state,  775, reg_775);
	dib8000_write_word(state,  776, reg_776);
	dib8000_write_word(state,  900, reg_900);
    dib8000_write_word(state, 1280, reg_1280);
}

int dib8000_enable_vbg_voltage(struct dibFrontend *fe)
{
	struct dib8000_state *state = fe->demod_priv;
	dbgpl(&dib8000_dbg, "enabling VBG voltage in the ADC");
	/* P_dual_adc_cfg0 */
	dib8000_write_word(state, 907, 0x0000);
	/* P_dual_adc_cfg1 = 3, P_sar_adc_cfg = 2 */
	dib8000_write_word(state, 908, (3 << 2) | (2 << 0));

	return DIB_RETURN_SUCCESS;
}
