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

#include "command.hpp"

namespace Slicer {

RemovePageCommand::RemovePageCommand(const Glib::RefPtr<Gio::ListStore<Page>>& pages,
                                     int position)
    : m_pages{pages}
    , m_position{static_cast<unsigned int>(position)}
    , m_removedPage{m_pages->get_item(static_cast<unsigned>(m_position))}
{
}

void RemovePageCommand::execute()
{
    m_pages->remove(m_position);
}

void RemovePageCommand::undo()
{
    m_pages->insert_sorted(m_removedPage, pageComparator{});
}

void RemovePageCommand::redo()
{
    execute();
}

RemovePagesCommand::RemovePagesCommand(const Glib::RefPtr<Gio::ListStore<Page>>& pages,
                                       const std::vector<unsigned int>& listPositions)
    : m_pages{pages}
    , m_listPositions{listPositions}
{
    for (auto position : m_listPositions) {
        auto page = m_pages->get_item(position);
        m_removedPages.push_back(page);
    }
}

void RemovePagesCommand::execute()
{
    // This is complicated.
    // The list positions are the positions in the list of the pages we
    // want to remove.
    // The problem is that, everytime a page is removed, all positions are invalidated.
    // After each page removal, the remaining positions must be decremented by one.
    for (unsigned int i = 0; i < m_listPositions.size(); ++i) {
        const unsigned int actualPosition = m_listPositions.at(i) - i;
        m_pages->remove(actualPosition);
    }
}

void RemovePagesCommand::undo()
{
    for (const auto& page : m_removedPages)
        m_pages->insert_sorted(page, pageComparator{});
}

void RemovePagesCommand::redo()
{
    execute();
}

RemovePageRangeCommand::RemovePageRangeCommand(const Glib::RefPtr<Gio::ListStore<Page>>& pages,
                                               int first,
                                               int last)
    : m_pages{pages}
    , m_first{first}
    , m_last{last}
{
    // Store pages in reversed order, since Gio::ListStore::splice()
    // inserts them in reversed order.
    for (int i = last; i >= first; --i)
        m_removedPages.push_back(m_pages->get_item(static_cast<unsigned>(i)));
}

void RemovePageRangeCommand::execute()
{
    const int nElem = m_last - m_first + 1;

    m_pages->splice(static_cast<unsigned>(m_first),
                    static_cast<unsigned>(nElem),
                    {});
}

void RemovePageRangeCommand::undo()
{
    m_pages->splice(static_cast<unsigned>(m_first), 0, m_removedPages);
}

void RemovePageRangeCommand::redo()
{
    execute();
}

RotatePagesRightCommand::RotatePagesRightCommand(const Glib::RefPtr<Gio::ListStore<Page>>& pages,
                                                 const std::vector<unsigned int>& pageNumbers,
                                                 sigc::signal<void, std::vector<unsigned int>>& pagesRotated)
    : m_pages{pages}
    , m_pageNumbers{pageNumbers}
    , m_pagesRotated{pagesRotated}
{
}

void RotatePagesRightCommand::execute()
{
    for (auto pageNumber : m_pageNumbers)
        m_pages->get_item(pageNumber)->rotateRight();

    m_pagesRotated.emit(m_pageNumbers);
}

void RotatePagesRightCommand::undo()
{
    for (auto pageNumber : m_pageNumbers)
        m_pages->get_item(pageNumber)->rotateLeft();

    m_pagesRotated.emit(m_pageNumbers);
}

void RotatePagesRightCommand::redo()
{
    execute();
}

RotatePagesLeftCommand::RotatePagesLeftCommand(const Glib::RefPtr<Gio::ListStore<Page>>& pages,
                                               const std::vector<unsigned int>& pageNumbers,
                                               sigc::signal<void, std::vector<unsigned int>>& pagesRotated)
    : m_pages{pages}
    , m_pageNumbers{pageNumbers}
    , m_pagesRotated{pagesRotated}
{
}

void RotatePagesLeftCommand::execute()
{
    for (auto pageNumber : m_pageNumbers)
        m_pages->get_item(pageNumber)->rotateLeft();

    m_pagesRotated.emit(m_pageNumbers);
}

void RotatePagesLeftCommand::undo()
{
    for (auto pageNumber : m_pageNumbers)
        m_pages->get_item(pageNumber)->rotateRight();

    m_pagesRotated.emit(m_pageNumbers);
}

void RotatePagesLeftCommand::redo()
{
    execute();
}

} // namespace Slicer
