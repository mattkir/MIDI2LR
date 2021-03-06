/*
  ==============================================================================

  CommandTableModel.cpp

This file is part of MIDI2LR. Copyright 2015 by Rory Jaffe.

MIDI2LR is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

MIDI2LR is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
MIDI2LR.  If not, see <http://www.gnu.org/licenses/>.
  ==============================================================================
*/
#include "CommandTableModel.h"

#include <exception>
#include <sstream>
#include <utility>

#include "CommandMenu.h"
#include "Misc.h"

CommandTableModel::CommandTableModel(const CommandSet& command_set, Profile& profile) noexcept
    : command_set_{command_set}, profile_{profile}
{
}

int CommandTableModel::getNumRows()
{
   try {
      // This must return the number of rows currently in the table.

      // If the number of rows changes, you must call TableListBox::updateContent()
      // to cause it to refresh the list.
      return gsl::narrow_cast<int>(profile_.Size());
   }
   catch (const std::exception& e) {
      rsj::ExceptionResponse(typeid(this).name(), __func__, e);
      throw;
   }
}

void CommandTableModel::paintCell(
    juce::Graphics& g, int row_number, int column_id, int width, int height, bool /*rowIsSelected*/)
{
   try {
      // This must draw one of the cells.

      // The graphics context's origin will already be set to the top-left of the
      // cell, whose size is specified by(width, height).

      // Note that the rowNumber value may be greater than the number of rows in your
      // list, so be careful that you don't assume it's less than getNumRows().
      g.setColour(juce::Colours::black);
      g.setFont(12.0f);
      if (column_id == 1) { // write the MIDI message in the MIDI command column
         if (profile_.Size() <= gsl::narrow_cast<size_t>(row_number)) { // error condition
            g.drawText("Unknown control", 0, 0, width, height, juce::Justification::centred);
            rsj::Log("Unknown control CommandTableModel::paintCell. "
                     + juce::String(profile_.Size())
                     + " rows in profile, row number to be painted is " + juce::String(row_number)
                     + '.');
         }
         else {
            std::ostringstream format_str;
            switch (
                const auto cmd = profile_.GetMessageForNumber(gsl::narrow_cast<size_t>(row_number));
                cmd.msg_id_type) {
            case rsj::MsgIdEnum::kNote:
               format_str << cmd.channel << " | Note : " << cmd.data;
               break;
            case rsj::MsgIdEnum::kCc:
               format_str << cmd.channel << " | CC: " << cmd.data;
               break;
            case rsj::MsgIdEnum::kPitchBend:
               format_str << cmd.channel << " | Pitch Bend";
               break;
            }
            g.drawText(format_str.str(), 0, 0, width, height, juce::Justification::centredLeft);
         }
      }
   }
   catch (const std::exception& e) {
      rsj::ExceptionResponse(typeid(this).name(), __func__, e);
      throw;
   }
}

void CommandTableModel::paintRowBackground(juce::Graphics& g,
    int /*rowNumber*/, //-V2009 overridden method
    int /*width*/, int /*height*/, bool row_is_selected)
{
   try {
      // This must draw the background behind one of the rows in the table.

      // The graphics context has its origin at the row's top-left, and your method
      // should fill the area specified by the width and height parameters.

      // Note that the rowNumber value may be greater than the number of rows in your
      // list, so be careful that you don't assume it's less than getNumRows().
      if (row_is_selected)
         g.fillAll(juce::Colours::lightblue);
   }
   catch (const std::exception& e) {
      rsj::ExceptionResponse(typeid(this).name(), __func__, e);
      throw;
   }
}

juce::Component* CommandTableModel::refreshComponentForCell(int row_number, int column_id,
    bool /*isRowSelected*/, juce::Component* existing_component_to_update)
{
   try {
      // This is used to create or update a custom component to go in a cell.

      // Any cell may contain a custom component, or can just be drawn with the
      // paintCell() method and handle mouse clicks with cellClicked().

      // This method will be called whenever a custom component might need to be
      // updated - e.g. when the table is changed, or TableListBox::updateContent()
      // is called.

      // If you don't need a custom component for the specified cell, then return
      // nullptr. (Bear in mind that even if you're not creating a new component,
      // you may still need to delete existingComponentToUpdate if it's non-null).

      // If you do want a custom component, and the existingComponentToUpdate is
      // null, then this method must create a new component suitable for the cell,
      // and return it.

      // If the existingComponentToUpdate is non - null, it will be a pointer to a
      // component previously created by this method.In this case, the method must
      // either update it to make sure it's correctly representing the given
      // cell(which may be different from the one that the component was created
      // for), or it can delete this component and return a new one.
      //// Because Juce recycles these components when scrolling, we need to reset
      //// their properties
      if (column_id == 2) // LR command column
      {
         auto command_select = dynamic_cast<CommandMenu*>(existing_component_to_update);

         // create a new command menu
         if (command_select == nullptr) {
#pragma warning(suppress : 26400 26409 24623 24624)
            command_select =
                new CommandMenu{profile_.GetMessageForNumber(gsl::narrow_cast<size_t>(row_number)),
                    command_set_, profile_};
         }
         else
            command_select->SetMsg(
                profile_.GetMessageForNumber(gsl::narrow_cast<size_t>(row_number)));

         // add 1 because 0 is reserved for no selection
         command_select->SetSelectedItem(
             command_set_.CommandTextIndex(profile_.GetCommandForMessage(
                 profile_.GetMessageForNumber(gsl::narrow_cast<size_t>(row_number))))
             + 1);

         return command_select;
      }
      return nullptr;
   }
   catch (const std::exception& e) {
      rsj::ExceptionResponse(typeid(this).name(), __func__, e);
      throw;
   }
}

void CommandTableModel::sortOrderChanged(int new_sort_column_id, bool is_forwards)
{
   try {
      // This callback is made when the table's sort order is changed.

      // This could be because the user has clicked a column header, or because the
      // TableHeaderComponent::setSortColumnId() method was called.

      // If you implement this, your method should re - sort the table using the
      // given column as the key.
      const auto current_sort = std::make_pair(new_sort_column_id, is_forwards);
      profile_.Resort(current_sort);
   }
   catch (const std::exception& e) {
      rsj::ExceptionResponse(typeid(this).name(), __func__, e);
      throw;
   }
}