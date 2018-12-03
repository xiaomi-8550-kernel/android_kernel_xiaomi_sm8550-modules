/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: Implements VDEV MLME public APIs
 */

#include <wlan_objmgr_vdev_obj.h>
#include <wlan_mlme_dbg.h>
#include "include/wlan_vdev_mlme.h"
#include "../../core/src/vdev_mlme_sm.h"
#include <wlan_vdev_mlme_api.h>
#include <qdf_module.h>

struct vdev_mlme_obj *wlan_vdev_mlme_get_cmpt_obj(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme;

	if (!vdev) {
		mlme_err("vdev is NULL");
		return NULL;
	}

	vdev_mlme = wlan_objmgr_vdev_get_comp_private_obj(vdev,
							  WLAN_UMAC_COMP_MLME);
	if (!vdev_mlme) {
		mlme_err(" MLME component object is NULL");
		return NULL;
	}

	return vdev_mlme;
}

void *wlan_vdev_mlme_get_ext_hdl(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (vdev_mlme)
		return vdev_mlme->ext_vdev_ptr;

	return NULL;
}

qdf_export_symbol(wlan_vdev_mlme_get_ext_hdl);

#ifdef CMN_VDEV_MLME_SM_ENABLE
QDF_STATUS wlan_vdev_mlme_sm_deliver_evt(struct wlan_objmgr_vdev *vdev,
					 enum wlan_vdev_sm_evt event,
					 uint16_t event_data_len,
					 void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme;
	QDF_STATUS status;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("vdev component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_vdev_sm_spin_lock(vdev_mlme);
	status = mlme_vdev_sm_deliver_event(vdev_mlme, event, event_data_len,
					    event_data);
	mlme_vdev_sm_spin_unlock(vdev_mlme);

	return status;
}

qdf_export_symbol(wlan_vdev_mlme_sm_deliver_evt);

QDF_STATUS wlan_vdev_mlme_sm_deliver_evt_sync(struct wlan_objmgr_vdev *vdev,
					      enum wlan_vdev_sm_evt event,
					      uint16_t event_data_len,
					      void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme;
	QDF_STATUS status;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("vdev component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	status = mlme_vdev_sm_deliver_event(vdev_mlme, event, event_data_len,
					    event_data);

	return status;
}

qdf_export_symbol(wlan_vdev_mlme_sm_deliver_evt_sync);

#ifdef SM_ENG_HIST_ENABLE
void wlan_vdev_mlme_sm_history_print(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("vdev component object is NULL");
		return;
	}

	mlme_vdev_sm_history_print(vdev_mlme);
}
#endif

QDF_STATUS wlan_vdev_allow_connect_n_tx(struct wlan_objmgr_vdev *vdev)
{
	enum wlan_vdev_state state;
	enum wlan_vdev_state substate;

	state = wlan_vdev_mlme_get_state(vdev);
	substate = wlan_vdev_mlme_get_substate(vdev);
	if ((state == WLAN_VDEV_S_UP) ||
	    ((state == WLAN_VDEV_S_SUSPEND) &&
	     (substate == WLAN_VDEV_SS_SUSPEND_CSA_RESTART)))
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wlan_vdev_mlme_is_active(struct wlan_objmgr_vdev *vdev)
{
	enum wlan_vdev_state state;

	state = wlan_vdev_mlme_get_state(vdev);
	if ((state == WLAN_VDEV_S_UP) ||  (state == WLAN_VDEV_S_DFS_CAC_WAIT) ||
	    (state == WLAN_VDEV_S_SUSPEND))
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}

qdf_export_symbol(wlan_vdev_mlme_is_active);

QDF_STATUS wlan_vdev_chan_config_valid(struct wlan_objmgr_vdev *vdev)
{
	enum wlan_vdev_state state;
	enum wlan_vdev_state substate;

	state = wlan_vdev_mlme_get_state(vdev);
	substate = wlan_vdev_mlme_get_substate(vdev);
	if (!((state == WLAN_VDEV_S_INIT) || (state == WLAN_VDEV_S_STOP)))
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wlan_vdev_is_going_down(struct wlan_objmgr_vdev *vdev)
{
	enum wlan_vdev_state state;
	enum wlan_vdev_state substate;

	state = wlan_vdev_mlme_get_state(vdev);
	substate = wlan_vdev_mlme_get_substate(vdev);
	if ((state == WLAN_VDEV_S_STOP) ||
	    ((state == WLAN_VDEV_S_SUSPEND) &&
	     (substate == WLAN_VDEV_SS_SUSPEND_SUSPEND_DOWN)))
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}
#endif
