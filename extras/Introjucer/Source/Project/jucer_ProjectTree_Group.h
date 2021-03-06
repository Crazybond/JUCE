/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

class GroupItem   : public ProjectTreeItemBase
{
public:
    GroupItem (const Project::Item& projectItem)
        : ProjectTreeItemBase (projectItem)
    {
    }

    bool isRoot() const                                 { return item.isMainGroup(); }
    bool acceptsFileDrop (const StringArray&) const     { return true; }

    void addNewGroup()
    {
        Project::Item newGroup (item.addNewSubGroup ("New Group", 0));
        triggerAsyncRename (newGroup);
    }

    bool acceptsDragItems (const OwnedArray<Project::Item>& selectedNodes)
    {
        for (int i = selectedNodes.size(); --i >= 0;)
            if (item.canContain (*selectedNodes.getUnchecked(i)))
                return true;

        return false;
    }

    void addFiles (const StringArray& files, int insertIndex)
    {
        for (int i = 0; i < files.size(); ++i)
        {
            const File file (files[i]);

            if (item.addFile (file, insertIndex, true))
                ++insertIndex;
        }
    }

    void moveSelectedItemsTo (OwnedArray<Project::Item>& selectedNodes, int insertIndex)
    {
        moveItems (selectedNodes, item, insertIndex);
    }

    void checkFileStatus()
    {
        for (int i = 0; i < getNumSubItems(); ++i)
            if (ProjectTreeItemBase* p = dynamic_cast<ProjectTreeItemBase*> (getSubItem(i)))
                p->checkFileStatus();
    }

    ProjectTreeItemBase* createSubItem (const Project::Item& child)
    {
        if (child.isGroup())   return new GroupItem (child);
        if (child.isFile())    return new SourceFileItem (child);

        jassertfalse;
        return nullptr;
    }

    void showDocument()
    {
        if (ProjectContentComponent* pcc = getProjectContentComponent())
            pcc->setEditorComponent (new GroupInformationComponent (item), nullptr);
    }

    static void openOrCloseAllSubGroups (TreeViewItem& item, bool shouldOpen)
    {
        item.setOpen (shouldOpen);

        for (int i = item.getNumSubItems(); --i >= 0;)
            if (TreeViewItem* sub = item.getSubItem(i))
                openOrCloseAllSubGroups (*sub, shouldOpen);
    }

    static void setFilesToCompile (Project::Item item, const bool shouldCompile)
    {
        if (item.isFile())
            item.getShouldCompileValue() = shouldCompile;

        for (int i = item.getNumChildren(); --i >= 0;)
            setFilesToCompile (item.getChild (i), shouldCompile);
    }

    void showPopupMenu()
    {
        PopupMenu m;
        addCreateFileMenuItems (m);

        m.addSeparator();

        if (isOpen())
            m.addItem (1, "Collapse all Sub-groups");
        else
            m.addItem (2, "Expand all Sub-groups");

        m.addSeparator();
        m.addItem (3, "Enable compiling of all enclosed files");
        m.addItem (4, "Disable compiling of all enclosed files");

        m.addSeparator();
        m.addItem (5, "Sort Items Alphabetically");
        m.addItem (6, "Sort Items Alphabetically (Groups first)");
        m.addSeparator();
        m.addItem (7, "Rename...");

        if (! isRoot())
            m.addItem (8, "Delete");

        launchPopupMenu (m);
    }

    void handlePopupMenuResult (int resultCode)
    {
        switch (resultCode)
        {
            case 1:     openOrCloseAllSubGroups (*this, false); break;
            case 2:     openOrCloseAllSubGroups (*this, true); break;
            case 3:     setFilesToCompile (item, true); break;
            case 4:     setFilesToCompile (item, false); break;
            case 5:     item.sortAlphabetically (false); break;
            case 6:     item.sortAlphabetically (true); break;
            case 7:     triggerAsyncRename (item); break;
            case 8:     deleteAllSelectedItems(); break;
            default:    processCreateFileMenuItem (resultCode); break;
        }
    }

    void addCreateFileMenuItems (PopupMenu& m)
    {
        m.addItem (1001, "Add New Group");
        m.addItem (1002, "Add Existing Files...");

        m.addSeparator();
        NewFileWizard().addWizardsToMenu (m);
    }

    void processCreateFileMenuItem (int menuID)
    {
        switch (menuID)
        {
            case 1001:  addNewGroup(); break;
            case 1002:  browseToAddExistingFiles(); break;

            default:
                NewFileWizard().runWizardFromMenu (menuID, item);
                break;
        }
    }
};
