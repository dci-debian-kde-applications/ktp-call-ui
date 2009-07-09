/*
    Copyright (C) 2009  George Kiagiadakis <kiagiadakis.george@gmail.com>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef GROUPMEMBERSMODEL_H
#define GROUPMEMBERSMODEL_H

#include "treemodel.h"
#include <TelepathyQt4/Channel>

class KCALLPRIVATE_EXPORT GroupMembersModel : public TreeModel
{
    Q_OBJECT
public:
    explicit GroupMembersModel(const Tp::ChannelPtr & channel, QObject *parent = 0);
    virtual ~GroupMembersModel();

private Q_SLOTS:
    void onGroupMembersChanged( const Tp::Contacts & groupMembersAdded,
                                const Tp::Contacts & groupLocalPendingMembersAdded,
                                const Tp::Contacts & groupRemotePendingMembersAdded,
                                const Tp::Contacts & groupMembersRemoved,
                                const Tp::Channel::GroupMemberChangeDetails & details);

private:
    void addContact(const Tp::ContactPtr & contact, KCall::GroupMembersListType listType);
    void removeContact(const Tp::ContactPtr & contact);

    struct Private;
    Private *const d;
};

#endif