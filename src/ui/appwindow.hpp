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

#ifndef SLICERWINDOW_HPP
#define SLICERWINDOW_HPP

#include "../application/backgroundthread.hpp"
#include "documentrenderer.hpp"
#include "headerbar.hpp"
#include "previewwindow.hpp"
#include "savingrevealer.hpp"
#include "welcomescreen.hpp"
#include <gtkmm/applicationwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/overlay.h>
#include <gtkmm/revealer.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stack.h>

namespace Slicer {

class AppWindow : public Gtk::ApplicationWindow {
public:
    AppWindow(BackgroundThread& bacgkroundThread);
    virtual ~AppWindow() override = default;

    void openDocument(const Glib::RefPtr<Gio::File>& file);

protected:
    virtual bool on_delete_event(GdkEventAny*) override;

private:
    std::unique_ptr<Document> m_document;
    std::atomic<bool> m_isSavingDocument{false};
    BackgroundThread& m_backgroundThread;

    HeaderBar m_headerBar;
    Gtk::Overlay m_overlay;
    Gtk::Stack m_stack;
    WelcomeScreen m_welcomeScreen;
    Gtk::ScrolledWindow m_scroller;
    View m_view;
    DocumentRenderer m_renderer;
    ActionBar m_actionBar;

    ZoomLevelWithActions m_zoomLevel;
    static const std::set<int> zoomLevels;

    SavingRevealer m_savingRevealer;
    Glib::Dispatcher m_savedDispatcher;
    Glib::Dispatcher m_savingFailedDispatcher;

    // Actions
    Glib::RefPtr<Gio::SimpleAction> m_openAction;
    Glib::RefPtr<Gio::SimpleAction> m_saveAction;
    Glib::RefPtr<Gio::SimpleAction> m_undoAction;
    Glib::RefPtr<Gio::SimpleAction> m_redoAction;
    Glib::RefPtr<Gio::SimpleAction> m_removeSelectedAction;
    Glib::RefPtr<Gio::SimpleAction> m_removeUnselectedAction;
    Glib::RefPtr<Gio::SimpleAction> m_removePreviousAction;
    Glib::RefPtr<Gio::SimpleAction> m_removeNextAction;
    Glib::RefPtr<Gio::SimpleAction> m_rotateRightAction;
    Glib::RefPtr<Gio::SimpleAction> m_rotateLeftAction;
    Glib::RefPtr<Gio::SimpleAction> m_previewPageAction;
    Glib::RefPtr<Gio::SimpleAction> m_cancelSelectionAction;

    // Functions
    void addActions();
    void setupWidgets();
    void setupSignalHandlers();
    void loadCustomCSS();
    void disableEditingActions();
    void enableEditingActions();

    // Callbacks
    void onSaveAction();
    void onOpenAction();
    void onUndoAction();
    void onRedoAction();
    void onRemoveSelectedPages();
    void onRemoveUnselectedPages();
    void onRemovePreviousPages();
    void onRemoveNextPages();
    void onRotatePagesRight();
    void onRotatePagesLeft();
    void onPreviewPage();
    void onCancelSelection();
    void onSelectedPagesChanged();
    void onCommandExecuted();
};
}

#endif // SLICERWINDOW_HPP
