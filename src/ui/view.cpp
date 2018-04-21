#include "view.hpp"
#include "viewchild.hpp"
#include "previewwindow.hpp"

namespace Slicer {

const int View::numRendererThreads = 1; // Poppler can't handle more than one
const std::set<int> View::zoomLevels = {200, 300, 400};

View::View(Gio::ActionMap& actionMap)
    : m_actionMap{actionMap}
    , m_zoomLevel{zoomLevels, m_actionMap}
{
    set_column_spacing(10);
    set_row_spacing(20);

    set_selection_mode(Gtk::SELECTION_SINGLE);
    set_activate_on_single_click(false);

    addActions();
    setupSignalHandlers();
}

View::~View()
{
    if (m_pageRendererPool != nullptr)
        m_pageRendererPool->stop();

    m_actionMap.remove_action("remove-selected");
    m_actionMap.remove_action("remove-previous");
    m_actionMap.remove_action("remove-next");
    m_actionMap.remove_action("preview-selected");
}

void View::setDocument(Document& document)
{
    stopRendering();
    m_document = &document;
    startGeneratingThumbnails(m_zoomLevel.minLevel());
}

void View::addActions()
{
    m_removeSelectedAction = m_actionMap.add_action("remove-selected", sigc::mem_fun(*this, &View::removeSelectedPage));
    m_removePreviousAction = m_actionMap.add_action("remove-previous", sigc::mem_fun(*this, &View::removePreviousPages));
    m_removeNextAction = m_actionMap.add_action("remove-next", sigc::mem_fun(*this, &View::removeNextPages));
    m_previewPageAction = m_actionMap.add_action("preview-selected", sigc::mem_fun(*this, &View::previewPage));
    m_removeSelectedAction->set_enabled(false);
    m_removePreviousAction->set_enabled(false);
    m_removeNextAction->set_enabled(false);
    m_previewPageAction->set_enabled(false);
}

void View::setupSignalHandlers()
{
    m_zoomLevel.changed.connect([this](int targetThumbnailSize) {
        stopRendering();
        startGeneratingThumbnails(targetThumbnailSize);
    });

    signal_selected_children_changed().connect([this]() {
        manageActionsEnabledStates();
    });

    signal_child_activated().connect([this](Gtk::FlowBoxChild*) {
        m_previewPageAction->activate();
    });
}

void View::manageActionsEnabledStates()
{
    const unsigned long numSelected = get_selected_children().size();

    if (numSelected == 0) {
        m_removeSelectedAction->set_enabled(false);
        m_removePreviousAction->set_enabled(false);
        m_removeNextAction->set_enabled(false);
    }
    else {
        m_removeSelectedAction->set_enabled();
    }

    if (numSelected == 1) {
        m_previewPageAction->set_enabled();

        const int index = get_selected_children().at(0)->get_index();
        if (index == 0)
            m_removePreviousAction->set_enabled(false);
        else
            m_removePreviousAction->set_enabled();

        if (index == static_cast<int>(get_children().size()) - 1)
            m_removeNextAction->set_enabled(false);
        else
            m_removeNextAction->set_enabled();
    }
    else {
        m_previewPageAction->set_enabled(false);
    }
}

void View::waitForRenderCompletion()
{
    if (m_pageRendererPool != nullptr)
        m_pageRendererPool->stop(true);

    m_pageRendererPool = std::make_unique<ctpl::thread_pool>(numRendererThreads);
}

void View::stopRendering()
{
    if (m_pageRendererPool != nullptr)
        m_pageRendererPool->stop();

    m_pageRendererPool = std::make_unique<ctpl::thread_pool>(numRendererThreads);
}

void View::startGeneratingThumbnails(int targetThumbnailSize)
{
    bind_list_store(m_document->pages(), [this, targetThumbnailSize](const Glib::RefPtr<Page>& page) {
        return Gtk::manage(new Slicer::ViewChild{page,
                                                 targetThumbnailSize,
                                                 *m_pageRendererPool});
    });
}

void View::removeSelectedPage()
{
    Gtk::FlowBoxChild* child = get_selected_children().at(0);
    const int index = child->get_index();
    m_document->removePage(index);
}

void View::removePreviousPages()
{
    std::vector<Gtk::FlowBoxChild*> selected = get_selected_children();

    if (selected.size() != 1)
        throw std::runtime_error(
            "Tried to remove previous pages with more "
            "than one page selected. This should never happen!");

    const int index = selected.at(0)->get_index();

    m_document->removePageRange(0, index - 1);
}

void View::removeNextPages()
{
    std::vector<Gtk::FlowBoxChild*> selected = get_selected_children();

    if (selected.size() != 1)
        throw std::runtime_error(
            "Tried to remove next pages with more "
            "than one page selected. This should never happen!");

    const int index = selected.at(0)->get_index();

    m_document->removePageRange(index + 1,
                                static_cast<int>(m_document->pages()->get_n_items()) - 1);
}

void View::previewPage()
{
    // We need to wait till the thumbnails finish rendering
    // before rendering a big preview, to prevent crashes.
    // Poppler isn't designed for rendering many pages of
    // the same document in different threads.
    waitForRenderCompletion();

    const int pageNumber = get_selected_children().at(0)->get_index();

    Glib::RefPtr<Slicer::Page> page
        = m_document->pages()->get_item(static_cast<unsigned>(pageNumber));

    (new Slicer::PreviewWindow{page})->show();
}
}