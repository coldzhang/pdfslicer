// PDF Slicer
// Copyright (C) 2017-2018 Julián Unrrein

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "actionbar.hpp"
#include <gtkmm/box.h>
#include <glibmm/i18n.h>

namespace Slicer {

ActionBar::ActionBar()
{
    auto rotateBox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL});
    rotateBox->get_style_context()->add_class("linked");

    m_buttonRotateLeft.set_image_from_icon_name("object-rotate-left-symbolic");
    m_buttonRotateLeft.set_tooltip_text(_("Rotate left"));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(m_buttonRotateLeft.gobj()), "win.rotate-left"); // NOLINT
    rotateBox->pack_start(m_buttonRotateLeft);

    m_buttonRotateRight.set_image_from_icon_name("object-rotate-right-symbolic");
    m_buttonRotateRight.set_tooltip_text(_("Rotate right"));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(m_buttonRotateRight.gobj()), "win.rotate-right"); // NOLINT
    rotateBox->pack_start(m_buttonRotateRight);

    auto removeBox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL});
    removeBox->get_style_context()->add_class("linked");

    m_buttonRemovePages.set_label(_("Remove selected"));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(m_buttonRemovePages.gobj()), "win.remove-selected"); // NOLINT
    removeBox->pack_start(m_buttonRemovePages);

    m_buttonRemoveUnselected.set_label(_("Remove unselected"));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(m_buttonRemoveUnselected.gobj()), "win.remove-unselected"); // NOLINT
    removeBox->pack_start(m_buttonRemoveUnselected);

    m_buttonRemovePrevious.set_label(_("Remove previous"));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(m_buttonRemovePrevious.gobj()), "win.remove-previous"); // NOLINT
    removeBox->pack_start(m_buttonRemovePrevious);

    m_buttonRemoveNext.set_label(_("Remove next"));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(m_buttonRemoveNext.gobj()), "win.remove-next"); // NOLINT
    removeBox->pack_start(m_buttonRemoveNext);

    m_buttonCancelSelection.set_label(_("Cancel selection"));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(m_buttonCancelSelection.gobj()), "win.cancel-selection"); // NOLINT

    pack_start(*rotateBox);
    pack_start(*removeBox);
    pack_end(m_buttonCancelSelection);

    // The following is to work around a bug with themes other than Adwaita.
    // Without this, the ActionBar is too narrow and the buttons don't fit.
    set_size_request(-1, 45);
}

} // namespace Slicer
