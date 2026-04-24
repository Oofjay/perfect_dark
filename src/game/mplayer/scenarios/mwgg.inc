/**
 * Combat
 *
 * This is a general combat scenario.
 *
 * There are no callback functions or special logic.
 */

struct defaultobj *chosencandidate = NULL;

struct weaponobj g_MwggTokenObj;

void mwgg_init(void)
{
	g_ScenarioData.mwgg.token = NULL;
}

s32 mwgg_num_props(void)
{
	return 1;
}

void mwgg_add_pad(s16 padnum)
{
	struct scenariodata_mwgg *data = &g_ScenarioData.mwgg;

#if VERSION >= VERSION_NTSC_1_0
	if (data->nextindex < ARRAYCOUNT(data->padnums))
#endif
	{
		osSyncPrintf("CaptureTheBriefcaseAddBankPad -> Adding New Pad %d  - Pad Id = %d-> Saving Pad\n", data->nextindex, padnum);

		data->padnums[data->nextindex] = padnum;
		data->nextindex++;
	}
}

void mwgg_reset(void)
{
	s32 i;

	g_ScenarioData.mwgg.nextindex = 0;

	for (i = 0; i < ARRAYCOUNT(g_ScenarioData.mwgg.padnums); i++) {
		g_ScenarioData.mwgg.padnums[i] = -1;
	}
}

void mwgg_create_token(void)
{
	char text[60];

	struct weaponobj template = {
		256,                    // extrascale
		0,                      // hidden2
		OBJTYPE_WEAPON,         // type
		MODEL_CHRDY357TRENT,     // modelnum
		0,                      // pad
		OBJFLAG_FALL | OBJFLAG_INVINCIBLE | OBJFLAG_FORCENOBOUNCE,
		OBJFLAG2_IMMUNETOGUNFIRE | OBJFLAG2_IMMUNETOEXPLOSIONS,
		0,                      // flags3
		NULL,                   // prop
		NULL,                   // model
		1, 0, 0,                // realrot
		0, 1, 0,
		0, 0, 1,
		0,                      // hidden
		NULL,                   // geo
		NULL,                   // projectile
		0,                      // damage
		1000,                   // maxdamage
		0xff, 0xff, 0xff, 0x00, // shadecol
		0xff, 0xff, 0xff, 0x00, // nextcol
		0x0fff,                 // floorcol
		0,                      // tiles
		WEAPON_DY357LX,         // weaponnum
		0,                      // upgradewant
		0,                      // miscbyte
		FUNC_PRIMARY,           // gunfunc
		0,                      // fadeouttimer60
		-1,                     // dualweaponnum
		-1,                     // timer240
		NULL,                   // dualweapon
	};

	struct prop *prop = g_Vars.activeprops;
	struct defaultobj *obj;
	s32 count = 0;
	struct defaultobj *candidates[20];

	// Build a list of candidate objects to replace. Consider only weapons.
	while (prop && count < 20) {
		if (prop->type == PROPTYPE_WEAPON)
		{
			obj = prop->obj;
			
			candidates[count] = obj;
			count++;

			//sprintf(text, "Count: %d*", count);
			//hudmsg_create_with_flags(text, HUDMSGTYPE_DEFAULT, HUDMSGFLAG_ONLYIFALIVE);
		}

		prop = prop->next;
	}
	
	// Choose the candidate and remove it
	if (count > 0) {
		count = random() % count;

		chosencandidate = candidates[count];
		g_ScenarioData.mwgg.tokenpad = chosencandidate->pad;
		chosencandidate->hidden |= OBJHFLAG_DELETING;
		chosencandidate->hidden2 |= OBJH2FLAG_CANREGEN;
	} else if (g_ScenarioData.mwgg.nextindex > 0) {
		g_ScenarioData.mwgg.tokenpad = g_ScenarioData.mwgg.padnums[random() % g_ScenarioData.mwgg.nextindex];
	} else {
		g_ScenarioData.mwgg.tokenpad = 0;
	}
	
	// Set up the token
	g_MwggTokenObj = template;
	g_MwggTokenObj.base.pad = g_ScenarioData.mwgg.tokenpad;

	setup_place_weapon(&g_MwggTokenObj, 999);

	g_MwggTokenObj.base.hidden2 &= ~OBJH2FLAG_CANREGEN;

	g_ScenarioData.mwgg.token = g_MwggTokenObj.base.prop;

	if (g_ScenarioData.mwgg.token) {
		g_ScenarioData.mwgg.token->forcetick = true;
	}
}


void mwgg_init_props(void)
{
	chosencandidate = NULL;
	mwgg_create_token();
}

void mwgg_tick(void)
{
	/*char text[60];
	
	if (g_ScenarioData.mwgg.token != NULL)
	{
	  sprintf(text, "%d %d %d\n", g_ScenarioData.mwgg.token->pos.x, g_ScenarioData.mwgg.token->pos.y, g_ScenarioData.mwgg.token->pos.z);
	  hudmsg_create_with_flags(text, HUDMSGTYPE_DEFAULT, HUDMSGFLAG_ONLYIFALIVE);
	}*/


	s32 i;
	u32 prevplayernum = g_Vars.currentplayernum;
	struct prop *prop;

	if (chosencandidate && chosencandidate->prop) {
		if (g_ScenarioData.mwgg.token == NULL || g_ScenarioData.mwgg.token->type != PROPTYPE_WEAPON) {
			chosencandidate = NULL;
		} else {
			chosencandidate->prop->timetoregen = TICKS(1200);
		}
	}

	g_ScenarioData.mwgg.token = NULL;

	// Check if gun is on the ground
	prop = g_Vars.activeprops;

	while (prop) {
		if (prop->type == PROPTYPE_WEAPON) {
			struct weaponobj *weapon = prop->weapon;

			if (weapon->weaponnum == WEAPON_DY357LX) {
				g_ScenarioData.mwgg.token = prop;
			}
		}

		prop = prop->next;
	}

	// Check if a player is holding it
	if (g_ScenarioData.mwgg.token == NULL) {
		for (i = 0; i < PLAYERCOUNT(); i++) {
			set_current_player_num(i);

			if (inv_has_goldengun()) {
				g_ScenarioData.mwgg.token = g_Vars.currentplayer->prop;
				break;
			}
		}
	}

	set_current_player_num(prevplayernum);

	// Check if a simulant is holding it
	if (g_ScenarioData.mwgg.token == NULL) {
		for (i = PLAYERCOUNT(); i < g_MpNumChrs; i++) {
#if VERSION >= VERSION_NTSC_1_0
			if (g_MpAllChrPtrs[i]->prop && g_MpAllChrPtrs[i]->aibot->hasbriefcase)
#else
			if (g_MpAllChrPtrs[i]->aibot->hasbriefcase)
#endif
			{
				g_ScenarioData.mwgg.token = g_MpAllChrPtrs[i]->prop;
				break;
			}
		}
	}

	if (g_ScenarioData.mwgg.token == NULL) {
		mwgg_create_token();
	}

	if (g_ScenarioData.mwgg.token == NULL) {
		g_ScenarioData.mwgg.pos.x = 0;
		g_ScenarioData.mwgg.pos.y = 0;
		g_ScenarioData.mwgg.pos.z = 0;
	} else {
		struct coord *pos = &g_ScenarioData.mwgg.pos;
		pos->x = g_ScenarioData.mwgg.token->pos.x;
		pos->y = g_ScenarioData.mwgg.token->pos.y;
		pos->z = g_ScenarioData.mwgg.token->pos.z;
	}
}

Gfx *mwgg_radar_extra(Gfx *gdl)
{
	if (/*(g_MpSetup.options & MPOPTION_HTB_SHOWONRADAR) &&*/
			g_ScenarioData.mwgg.token != NULL &&
			g_ScenarioData.mwgg.token->type != PROPTYPE_PLAYER &&
			g_ScenarioData.mwgg.token->type != PROPTYPE_CHR) {
		struct coord dist;
		dist.x = g_ScenarioData.mwgg.pos.x - g_Vars.currentplayer->prop->pos.x;
		dist.y = (g_ScenarioData.mwgg.pos.y + 50) - g_Vars.currentplayer->prop->pos.y;
		dist.z = g_ScenarioData.mwgg.pos.z - g_Vars.currentplayer->prop->pos.z;
		gdl = radar_draw_dot(gdl, g_ScenarioData.mwgg.token, &dist, 0x00ff0000, 0, 1);
	}

	return gdl;
}

bool mwgg_radar_chr(Gfx **gdl, struct prop *prop)
{
	if (/*(g_MpSetup.options & MPOPTION_HTB_SHOWONRADAR) &&*/
			g_ScenarioData.mwgg.token &&
			prop == g_ScenarioData.mwgg.token) {
		if (prop->type == PROPTYPE_PLAYER || prop->type == PROPTYPE_CHR) {
			struct coord dist;
			dist.x = prop->pos.x - g_Vars.currentplayer->prop->pos.x;
			dist.y = (prop->pos.y + 50) - g_Vars.currentplayer->prop->pos.y;
			dist.z = prop->pos.z - g_Vars.currentplayer->prop->pos.z;

			if (g_MpSetup.options & MPOPTION_TEAMSENABLED) {
				u32 colour = g_TeamColours[radar_get_team_index(prop->chr->team)];
				*gdl = radar_draw_dot(*gdl, g_ScenarioData.mwgg.token, &dist, colour, 0, 1);
			} else {
				*gdl = radar_draw_dot(*gdl, g_ScenarioData.mwgg.token, &dist, 0x00ff0000, 0, 1);
			}

			return true;
		}
	}

	return false;
}

bool mwgg_highlight_prop(struct prop *prop, s32 *colour)
{
	//if ((g_MpSetup.options & MPOPTION_HTB_HIGHLIGHTBRIEFCASE) && prop == g_ScenarioData.htb.token) {
	if (prop == g_ScenarioData.mwgg.token) {
		colour[0] = 0;
		colour[1] = 0xff;
		colour[2] = 0;
		colour[3] = 0x40;

		return true;
	}

	return false;
}
