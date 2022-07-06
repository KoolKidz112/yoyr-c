#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"
#include "UserMessages.h"
#include "soundent.h"

LINK_ENTITY_TO_CLASS(weapon_penis, CPenis)

void CPenis::Spawn() {
	// Define the classname of our... super special phallic object weapon (That is not actually a dick,
	// but rather a super cool glock)
	pev->classname = MAKE_STRING( "weapon_penis" );

	// Precache stuff yeah
	Precache();

	// Set the weapon ID
	m_iId = WEAPON_PENIS;

	// Hay! Engine! This WORLDMODEL is something that can be attributed to WEAPON_PENIS
	SET_MODEL( ENT(pev), "models/w_9mmhandgun.mdl" );

	// Incredibly fucking obvious
	m_iDefaultAmmo = PENIS_DEFAULT_GIVE;

	// Fall to the ground when spawned
	FallInit();
}

void CPenis::Precache() {
	// Glock's models and sounds are already precached, so there is no need to redo it here
	// UNLESS...
	PRECACHE_MODEL("models/v_penis.mdl");
	PRECACHE_SOUND("weapons/pl_flurp.wav");
	// However it is still necessary to precache events...
	m_usFirePenis = PRECACHE_EVENT(1, "events/penis.sc");
	m_usFirePenisRocket = PRECACHE_EVENT(1, "events/penis2.sc");
}

BOOL CPenis::GetItemInfo(ItemInfo* p) {
	p->pszName = STRING(pev->classname);

	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;

	// Add a super cool secondary eventually
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;

	p->iMaxClip = PENIS_MAX_CLIP;

	p->iFlags = 0;
	p->iSlot = 1;
	p->iPosition = 2;

	p->iId = m_iId = WEAPON_PENIS;
	p->iWeight = PENIS_WEIGHT;

	return TRUE;
}

BOOL CPenis::AddToPlayer(CBasePlayer* pPlayer) {
	// Return true if weapon was picked up
	if (CBasePlayerWeapon::AddToPlayer(pPlayer)) {
		// Send message to client, showing the Wonderful pickup item !!!!!!11
		// MARVELOUS Marvelous
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
			WRITE_BYTE(m_iId);
		MESSAGE_END();
		return TRUE; // I Love Return Statements :D
	}
	return FALSE;
	//vvvvv
	//v v v
	// v v
	//  v
}

void CPenis::PrimaryAttack() {
	// Nobody gives a shit about whether or not you're firing underwater so don't take care of that

	// Check if the clip is empty
	if (m_iClip <= 0) {
		if (!m_fInReload && m_fFireOnEmpty) {
			// Player is a tenacious asshole and is holding down fire before and after the weapon is 
			// empty
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
		}
		return;
	}

	// Start Blasting
	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	//m_iClip = 1; // Increasing it would be cool but fuck you im not doing that

	// WTF DOES |= MEAN
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	// tf????
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	// Shoot da bullets
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);
	// OW JESUS FUCK I JUST POKED MY EYE OWWW WAHGHHG UFCK
	Vector vecDir = m_pPlayer->FireBulletsPlayer(
		1,
		vecSrc,
		vecAiming,
		VECTOR_CONE_1DEGREES,
		8192,
		BULLET_PLAYER_9MM,
		0,
		0,
		m_pPlayer->pev,
		m_pPlayer->random_seed
	);

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFirePenis, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, 0, 0, (m_iClip == 0) ? 1 : 0, 0);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0) {
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
	}

	// LMao
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.01;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}

void CPenis::SecondaryAttack() {
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 0)
	{
		PlayEmptySound();
		return;
	}

	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

	m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;
	m_pPlayer->m_flStopExtraSoundTime = UTIL_WeaponTimeBase() + 0.2;

	// m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	// we don't add in DEEZ NUTS
	CGrenade::ShootContact(m_pPlayer->pev,
		m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16,
		gpGlobals->v_forward * 800);

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT(flags, m_pPlayer->edict(), m_usFirePenisRocket);

	m_flNextPrimaryAttack = GetNextAttackDelay(0.01);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase();
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5;

	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
}

BOOL CPenis::Deploy() {
	return DefaultDeploy(
		"models/v_penis.mdl", // GITHUB PLS DONT BAN ME
		"models/p_9mmhandgun.mdl",
		GLOCK_DRAW, // Since this uses the glock model animations we can just use its precious ENUMs...........
		"onehanded",
		UseDecrement(),
		pev->body
	);
}

void CPenis::Holster(int skiplocal) {
	// Cancel any reload in progress
	m_fInReload = FALSE;

	// Delay the next player's attack for about the same time as the holster animation takes
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	// Dmc2
	SendWeaponAnim(GLOCK_HOLSTER);
}

void CPenis::Reload() {
	// Don't reload if the ppylyprfhkjoifgnlkjdfg
	if (m_pPlayer->ammo_9mm <= 0) return;

	int iResult;
	if (m_iClip == 0) iResult = DefaultReload(PENIS_MAX_CLIP, GLOCK_RELOAD, 1.5);
	else iResult = DefaultReload(PENIS_MAX_CLIP, GLOCK_RELOAD_NOT_EMPTY, 1.5);

	if (iResult) {
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
	}
}

void CPenis::WeaponIdle() {
	// In the suburbs I
	ResetEmptySound();
	// I learned to drive
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);
	// And you told me we'd never survive
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase() || m_iClip <= 0) return;
	if (m_iClip != 0)
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.0, 1.0);

		if (flRand <= 0.3 + 0 * 0.75)
		{
			iAnim = GLOCK_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 49.0 / 16;
		}
		else if (flRand <= 0.6 + 0 * 0.875)
		{
			iAnim = GLOCK_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 16.0;
		}
		else
		{
			iAnim = GLOCK_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
		}
		SendWeaponAnim(iAnim, 1);
	}
}