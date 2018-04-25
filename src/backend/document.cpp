#include "document.hpp"
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>
#include <giomm/file.h>
#include <podofo.h>
#include <range/v3/all.hpp>
#include <functional>
#include <iostream>

namespace Slicer {

Document::Document(std::string filePath)
    : m_sourcePath{std::move(filePath)}
{
    Glib::ustring uri = Glib::filename_to_uri(m_sourcePath);

    m_popplerDocument = poppler_document_new_from_file(uri.c_str(),
                                                       nullptr,
                                                       nullptr);

    if (m_popplerDocument == nullptr)
        throw std::runtime_error("Couldn't load file: " + m_sourcePath);

    const int num_pages = poppler_document_get_n_pages(m_popplerDocument);
    if (num_pages == 0)
        throw std::runtime_error("The file has zero pages");

    m_pages = Gio::ListStore<Page>::create();

    for (int i = 0; i < num_pages; ++i) {
        PopplerPage* popplerPage = poppler_document_get_page(m_popplerDocument, i);
        auto page = Glib::RefPtr<Page>{new Page{popplerPage}};
        m_pages->append(page);
    }
}

Document::~Document()
{
    g_object_unref(m_popplerDocument);
}

std::string getTempFilePath()
{
    const std::string tempDirectory = Glib::get_tmp_dir();
    const std::string tempFileName = "pdfslicer-temp.pdf";
    const std::vector<std::string> pathParts = {tempDirectory, tempFileName};
    const std::string tempFilePath = Glib::build_filename(pathParts);

    return tempFilePath;
}

void Document::saveDocument(const std::string& destinationPath) const
{
    const std::string tempFilePath = getTempFilePath();

    makePDFCopy(m_sourcePath, getTempFilePath());

    auto oldFile = Gio::File::create_for_path(destinationPath);
    auto newFile = Gio::File::create_for_path(tempFilePath);
    newFile->move(oldFile, Gio::FILE_COPY_OVERWRITE);
}

void Document::removePage(int pageNumber)
{
    auto command = std::make_shared<RemovePageCommand>(m_pages, pageNumber);
    m_commandManager.execute(command);
}

void Document::removePages(const std::vector<unsigned int>& positions)
{
    auto command = std::make_shared<RemovePagesCommand>(m_pages, positions);
    m_commandManager.execute(command);
}

void Document::removePageRange(int first, int last)
{
    auto command = std::make_shared<RemovePageRangeCommand>(m_pages, first, last);
    m_commandManager.execute(command);
}

void Document::makePDFCopy(const std::string& sourcePath,
                           const std::string& destinationPath) const
{
    PoDoFo::PdfMemDocument sourceDocument{sourcePath.c_str()};
    const std::vector<unsigned int> numbersToDelete = pageNumbersToDeleteOnSaving();

    for (unsigned int i = 0; i < numbersToDelete.size(); ++i)
        sourceDocument.DeletePages(static_cast<int>(numbersToDelete.at(i) - i),
                                   1);

    try {
        sourceDocument.Write(destinationPath.c_str());
    }
    catch (PoDoFo::PdfError& e) {
        std::cerr << e.what() << std::endl;

        const PoDoFo::TDequeErrorInfo& callStack = e.GetCallstack();

        if (callStack.empty())
            std::cerr << "The callstack is empty" << std::endl;
        else
            std::cerr << "The callstack has something:" << std::endl;

        //        for (const PoDoFo::PdfErrorInfo& info : callStack)
        //            std::cerr << info.GetLine() << std::endl;
        std::cerr << callStack.back().GetFilename() << " - Line " << callStack.back().GetLine() << std::endl;
    }
}

std::vector<unsigned int> Document::pageNumbers() const
{
    std::vector<unsigned int> numbers;

    for (unsigned int i = 0; i < m_pages->get_n_items(); ++i)
        numbers.push_back(static_cast<unsigned>(m_pages->get_item(i)->number()));

    return numbers;
}

std::vector<unsigned int> Document::pageNumbersToDeleteOnSaving() const
{
    std::vector<unsigned int> numbersToDelete;

    ranges::set_difference(ranges::view::iota(0, poppler_document_get_n_pages(m_popplerDocument)),
                           pageNumbers(),
                           ranges::back_inserter(numbersToDelete));

    return numbersToDelete;
}
}
