/**
 * Combat
 *
 * This is a general combat scenario.
 *
 * There are no callback functions or special logic.
 */

MenuItemHandlerResult menuhandler_mp_cr_interval(s32 operation, struct menuitem *item, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_GET_SLIDER_VALUE:
		data->slider.value = g_Vars.crinterval;
		break;
	case MENUOP_CONFIRM:
		g_Vars.crinterval = (u8)data->slider.value;
		break;
	case MENUOP_GET_SLIDER_LABEL:
		sprintf(data->slider.label, lang_get(L_MPMENU_371), data->slider.value + 10); // "%d Seconds"
		break;
	}

	return 0;
}

struct menuitem g_CrOptionsMenuItems[] = {
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_MPMENU_222, // "One-Hit Kills"
		MPOPTION_ONEHITKILLS,
		menuhandler_mp_one_hit_kills,
	},
	{
		MENUITEMTYPE_DROPDOWN,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_MPMENU_223, // "Slow Motion"
		0,
		menuhandler_mp_slow_motion,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_MPMENU_224, // "Fast Movement"
		MPOPTION_FASTMOVEMENT,
		menuhandler_mp_checkbox_option,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_MPMENU_225, // "Display Team"
		MPOPTION_DISPLAYTEAM,
		menuhandler_mp_display_team,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_MPMENU_226, // "No Radar"
		MPOPTION_NORADAR,
		menuhandler_mp_checkbox_option,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_MPMENU_227, // "No Auto-Aim"
		MPOPTION_NOAUTOAIM,
		menuhandler_mp_checkbox_option,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_MPMENU_490, // "Dual Wielding"
		MPOPTION_CR_DUALWIELDING,
		menuhandler_mp_checkbox_option,
	},
	{
		MENUITEMTYPE_SLIDER,
		0,
		MENUITEMFLAG_LOCKABLEMINOR,
		L_MPMENU_492, // "Interval"
		0x0000006e,
		menuhandler_mp_cr_interval,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG,
		L_MPMENU_239, // "Back"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CrOptionsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_494, // "Casino Royale Options"
	g_CrOptionsMenuItems,
	mp_options_menu_dialog,
	MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

void cr_give_weapon(bool unequipprevious)
{
	s32 i;
	s32 mpweaponnum;
	s32 weaponnum;
	u8 previousweaponnum;
	u32 ammotype;
	u32 ammotype2;

	u32 prevplayernum = g_Vars.currentplayernum;

	g_ScenarioData.cr.maxoptions = 0;
	
	for (i = 0; i < NUM_MPWEAPONSLOTS; ++i) {
		// Make sure the current weapon isn't able to be chosen again, and disabled slots are skipped
		if (g_MpSetup.weapons[i] != MPWEAPON_DISABLED && g_MpSetup.weapons[i] != g_ScenarioData.cr.mpweaponindex) { 
			g_ScenarioData.cr.weaponoptions[g_ScenarioData.cr.maxoptions] = g_MpSetup.weapons[i];
			++g_ScenarioData.cr.maxoptions;
		}
	}
	
	previousweaponnum = g_ScenarioData.cr.weaponindex;
	mpweaponnum = g_ScenarioData.cr.weaponoptions[random() % g_ScenarioData.cr.maxoptions];
	g_ScenarioData.cr.mpweaponindex = mpweaponnum;
	
	if (mpweaponnum == MPWEAPON_NONE) { weaponnum = WEAPON_UNARMED; }
	else { weaponnum = g_MpWeapons[mpweaponnum].weaponnum; }
	g_ScenarioData.cr.weaponindex = weaponnum;
	
	ammotype = bgun_get_ammo_type_for_weapon(weaponnum, 0);
	ammotype2 = bgun_get_ammo_type_for_weapon(weaponnum, 1);
		
	for (i = 0; i < PLAYERCOUNT(); i++) {
		set_current_player_num(i);
		if (unequipprevious) { inv_remove_item_by_num(previousweaponnum); }
		
		if (g_MpSetup.options & MPOPTION_CR_DUALWIELDING) {
			inv_give_single_weapon(weaponnum);
			inv_give_double_weapon(weaponnum, weaponnum);
			bgun_equip_weapon2(HAND_RIGHT, weaponnum);
			bgun_equip_weapon2(HAND_LEFT, weaponnum);
		}
		else {
			inv_give_single_weapon(weaponnum);
			bgun_equip_weapon(weaponnum);
		}
		
		ammo_handle_pickup(ammotype, 99999, false, false);
		ammo_handle_pickup(ammotype2, 99999, false, false);
	}
	
	set_current_player_num(prevplayernum);

	for (i = PLAYERCOUNT(); i < g_MpNumChrs; i++) {
		if (unequipprevious) { botinv_remove_item(g_MpAllChrPtrs[i], previousweaponnum); }
		
		botinv_give_single_weapon(g_MpAllChrPtrs[i], weaponnum);
		if (g_MpSetup.options & MPOPTION_CR_DUALWIELDING && gset_has_weapon_flag(weaponnum, WEAPONFLAG_DUALWIELD)) { botinv_give_dual_weapon(g_MpAllChrPtrs[i], weaponnum); }
		botinv_switch_to_weapon(g_MpAllChrPtrs[i], weaponnum, 0);
		if (g_MpSetup.options & MPOPTION_CR_DUALWIELDING && gset_has_weapon_flag(weaponnum, WEAPONFLAG_DUALWIELD)) { botinv_switch_to_weapon(g_MpAllChrPtrs[i], weaponnum, 0); }
		
		botact_give_ammo_by_type(g_MpAllChrPtrs[i]->aibot, ammotype, 99999);
		botact_give_ammo_by_type(g_MpAllChrPtrs[i]->aibot, ammotype2, 99999);
	}
}

void cr_reset(void)
{
	g_ScenarioData.cr.allocatedammo = false;
	g_ScenarioData.cr.elapsed240 = 0;
	g_ScenarioData.cr.maxoptions = 0;
}

void cr_init_props(void)
{
	struct prop *prop = g_Vars.activeprops;
	struct defaultobj *obj;
		
	while (prop) {
		obj = prop->obj;
		
		if (obj != NULL) {
			if (prop->type == PROPTYPE_WEAPON || obj->type == OBJTYPE_MULTIAMMOCRATE) {
				obj->hidden |= OBJHFLAG_DELETING;
				obj->hidden2 &= ~OBJH2FLAG_CANREGEN;
			}
		}
		
		prop = prop->next;
	}

	//g_ScenarioData.cr.allocatedammo = false;
}

void cr_tick(void)
{
	if (g_ScenarioData.cr.allocatedammo == false)
	{
		cr_give_weapon(false);
		
		g_ScenarioData.cr.allocatedammo = true;
	}
	
	if (g_Vars.tickmode != TICKMODE_MPSWIRL) {
		g_ScenarioData.cr.elapsed240 += g_Vars.lvupdate240;
	}
	
	if (g_ScenarioData.cr.elapsed240 >= g_Vars.crinterval * TICKS(240) + TICKS(2400))
	{
		snd_start(var80095200, SFXNUM_05B8_MP_SCOREPOINT, 0, -1, -1, -1, -1, -1);
	
		cr_give_weapon(true);
		
		g_ScenarioData.cr.elapsed240 = 0;
	}
}

Gfx *cr_render_hud(Gfx *gdl)
{
	s32 time240;
	s32 mins;
	s32 secs;
	s32 textwidth;
	s32 textheight;
	s32 x;
	s32 y;
	struct chrdata *chr = g_Vars.currentplayer->prop->chr;
	char text[64];

	x = vi_get_view_left() + vi_get_view_width() / 2;
	y = vi_get_view_top() + 10;

	time240 = g_Vars.crinterval * TICKS(240) - g_ScenarioData.cr.elapsed240;
	time240 += VERSION >= VERSION_PAL_BETA ? TICKS(2640) - 1 : 2400;
	mins = time240 / TICKS(60 * 240);
	time240 -= TICKS(60 * 240) * mins;

#if VERSION >= VERSION_PAL_BETA
	secs = time240 / TICKS(240);
#else
	secs = (time240 + (TICKS(240) - 1)) / TICKS(240);
#endif

	if ((g_Vars.crinterval * 60 + 600) / 3600) {
		sprintf(text, "%d:%02d", mins, secs);
	} else {
		sprintf(text, "%02d", secs);
	}

	gdl = text_begin(gdl);
	text_measure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);

	x -= textwidth / 2;
	textwidth += x;
	textheight += y;

#if VERSION >= VERSION_NTSC_1_0
	gdl = text_draw_black_textbox(gdl, x, y, textwidth, textheight);
	gdl = text_render_v1(gdl, &x, &y, text, g_CharsNumeric, g_FontNumeric, 0x00ff00a0, 0xa0, vi_get_width(), vi_get_height(), 0, 0);
#else
	gdl = text_draw_black_box(gdl, &x, &y, &textwidth, &textheight);
	gdl = text_render_v1(gdl, &x, &y, text, g_CharsNumeric, g_FontNumeric, 0x00ff00a0, 0x88, vi_get_width(), vi_get_height(), 0, 0);
#endif
	gdl = text_end(gdl);

	return gdl;
}

void cr_read_save(struct savebuffer *buffer)
{
	g_Vars.crinterval = savebuffer_read_bits(buffer, 8);
}

void cr_write_save(struct savebuffer *buffer)
{
	savebuffer_write_bits(buffer, g_Vars.crinterval, 8);
}
