/** 
 * @file llfloatersidepanelcontainer.cpp
 * @brief LLFloaterSidePanelContainer class definition
 *
 * $LicenseInfo:firstyear=2011&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2011, Linden Research, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"

#include "llfloaterreg.h"
#include "llfloatersidepanelcontainer.h"
#include "llpaneleditwearable.h"

// newview includes
#include "llsidetraypanelcontainer.h"
#include "lltransientfloatermgr.h"
#include "llpaneloutfitedit.h"
#include "llsidepanelappearance.h"

//static
const std::string LLFloaterSidePanelContainer::sMainPanelName("main_panel");

LLFloaterSidePanelContainer::LLFloaterSidePanelContainer(const LLSD& key, const Params& params)
: LLFloater(key, params)
, mAppQuiting( false )
{
	// Prevent transient floaters (e.g. IM windows) from hiding
	// when this floater is clicked.
	LLTransientFloaterMgr::getInstance()->addControlView(LLTransientFloaterMgr::GLOBAL, this);
	//We want this container to handle the shutdown logic of the sidepanelappearance.
	mVerifyUponClose = TRUE;
}

BOOL LLFloaterSidePanelContainer::postBuild()
{
	setCloseConfirmationCallback( boost::bind(&LLFloaterSidePanelContainer::onConfirmationClose,this,_2));
	return TRUE;
}

void  LLFloaterSidePanelContainer::onConfirmationClose( const LLSD &confirm )
{
	mAppQuiting = confirm.asBoolean();
	onClickCloseBtn();
}


LLFloaterSidePanelContainer::~LLFloaterSidePanelContainer()
{
	LLTransientFloaterMgr::getInstance()->removeControlView(LLTransientFloaterMgr::GLOBAL, this);
}

void LLFloaterSidePanelContainer::onOpen(const LLSD& key)
{
	getChild<LLPanel>(sMainPanelName)->onOpen(key);
	mAppQuiting = false;
}

void LLFloaterSidePanelContainer::onClose( bool app_quitting )
{
	if (! mAppQuiting ) { mForceCloseAfterVerify = true; }
	LLSidepanelAppearance* panel = getSidePanelAppearance();
	if ( panel )
	{		
		panel->mRevertSet = true;
		panel->onCloseFromAppearance( this );			
	}
}

void LLFloaterSidePanelContainer::onClickCloseBtn()
{
	LLSidepanelAppearance* panel = getSidePanelAppearance();
	if ( panel )
	{
		panel->onClose( this );			
	}
	else
	{
		LLFloater::onClickCloseBtn();
	}
}
void LLFloaterSidePanelContainer::close()
{
	LLFloater::onClickCloseBtn();
}

LLPanel* LLFloaterSidePanelContainer::openChildPanel(const std::string& panel_name, const LLSD& params)
{
	LLView* view = findChildView(panel_name, true);
	if (!view) return NULL;

	if (!getVisible())
	{
		openFloater();
	}

	LLPanel* panel = NULL;

	LLSideTrayPanelContainer* container = dynamic_cast<LLSideTrayPanelContainer*>(view->getParent());
	if (container)
	{
		container->openPanel(panel_name, params);
		panel = container->getCurrentPanel();
	}
	else if ((panel = dynamic_cast<LLPanel*>(view)) != NULL)
	{
		panel->onOpen(params);
	}

	return panel;
}

void LLFloaterSidePanelContainer::showPanel(const std::string& floater_name, const LLSD& key)
{
	//If we're already open then check whether anything is dirty	 
	LLFloaterSidePanelContainer* floaterp = LLFloaterReg::getTypedInstance<LLFloaterSidePanelContainer>(floater_name);	
	if (floaterp)
	{
		if ( floaterp->getVisible() )
		{
			LLSidepanelAppearance* panel = floaterp->getSidePanelAppearance();
			if ( panel )
			{
				if ( panel->checkForDirtyEdits() )
				{
					panel->onClickConfirmExitWithoutSaveIntoAppearance( floaterp );
				}
				else
				{
					//or a call into some new f() that just shows inv panel?
					floaterp->openChildPanel(sMainPanelName, key);
				}
			}
		}
		else
		{
			floaterp->openChildPanel(sMainPanelName, key);
		}
	}
}

void LLFloaterSidePanelContainer::showPanel(const std::string& floater_name, const std::string& panel_name, const LLSD& key)
{
	LLFloaterSidePanelContainer* floaterp = LLFloaterReg::getTypedInstance<LLFloaterSidePanelContainer>(floater_name);
	if (floaterp)
	{
		floaterp->openChildPanel(panel_name, key);
	}
}

LLPanel* LLFloaterSidePanelContainer::getPanel(const std::string& floater_name, const std::string& panel_name)
{
	LLFloaterSidePanelContainer* floaterp = LLFloaterReg::getTypedInstance<LLFloaterSidePanelContainer>(floater_name);

	if (floaterp)
	{
		return floaterp->findChild<LLPanel>(panel_name, true);
	}

	return NULL;
}

LLSidepanelAppearance* LLFloaterSidePanelContainer::getSidePanelAppearance()
{
	LLSidepanelAppearance* panel_appearance = NULL;
	LLPanelOutfitEdit* panel_outfit_edit = dynamic_cast<LLPanelOutfitEdit*>(LLFloaterSidePanelContainer::getPanel("appearance", "panel_outfit_edit"));
	if (panel_outfit_edit)
	{
		LLFloater *parent = gFloaterView->getParentFloater(panel_outfit_edit);
		if (parent == this )
		{
			panel_appearance = dynamic_cast<LLSidepanelAppearance*>(getPanel("appearance"));
		}
	}
	return panel_appearance;			

}
