--[[============================================================================
@file    i2c.lua

@author  Daniel Zorychta

@brief   Configuration script for I2C module.

@note    Copyright (C) 2014 Daniel Zorychta <daniel.zorychta@gmail.com>

         This program is free software; you can redistribute it and/or modify
         it under the terms of the GNU General Public License as published by
         the  Free Software  Foundation;  either version 2 of the License, or
         any later version.

         This  program  is  distributed  in the hope that  it will be useful,
         but  WITHOUT  ANY  WARRANTY;  without  even  the implied warranty of
         MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the
         GNU General Public License for more details.

         You  should  have received a copy  of the GNU General Public License
         along  with  this  program;  if not,  write  to  the  Free  Software
         Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


==============================================================================]]
module(..., package.seeall)


--==============================================================================
-- EXTERNAL MODULES
--==============================================================================
require("wx")
require("modules/ctcore")


--==============================================================================
-- GLOBAL OBJECTS
--==============================================================================
-- module's main object
i2c = {}


--==============================================================================
-- LOCAL OBJECTS
--==============================================================================
local ui = {}
local ID = {}
local prio_list = ct:get_priority_list("stm32f1")
local MAX_NUMBER_OF_DEVICES = 8

--==============================================================================
-- LOCAL FUNCTIONS
--==============================================================================
--------------------------------------------------------------------------------
-- @brief  Function loads all controls from configuration scripts
-- @param  None
-- @return None
--------------------------------------------------------------------------------
local function load_controls(I2C)
        local module_enabled = ct:get_module_state("I2C")
        ui.CheckBox_enable:SetValue(module_enabled)

        local I2C        = ui.Choice_I2C:GetSelection() + 1
        local I2C_enable = ct:yes_no_to_bool(ct:key_read(config.arch.stm32f1.key["I2C"..I2C.."_ENABLE"]))
        local use_DMA    = ct:yes_no_to_bool(ct:key_read(config.arch.stm32f1.key["I2C"..I2C.."_USE_DMA"]))
        local IRQ_prio   = ct:key_read(config.arch.stm32f1.key["I2C"..I2C.."_IRQ_PRIO"])
        local SCL_freq   = tonumber(ct:key_read(config.arch.stm32f1.key["I2C"..I2C.."_SCL_FREQ"]))/1000
        local no_of_dev  = tonumber(ct:key_read(config.arch.stm32f1.key["I2C"..I2C.."_NUMBER_OF_DEVICES"]))

        ui.CheckBox_I2C_enable:SetValue(I2C_enable)
        ui.CheckBox_use_DMA:SetValue(use_DMA)
        ui.SpinCtrl_SCL_freq:SetValue(SCL_freq)
        ui.Choice_dev_no:SetSelection(no_of_dev - 1)

        if IRQ_prio == config.project.def.DEFAULT_IRQ_PRIORITY:GetValue() then
                IRQ_prio = #prio_list
        else
                IRQ_prio = math.floor(tonumber(IRQ_prio) / 16)
        end
        ui.Choice_IRQ_priority:SetSelection(IRQ_prio)

        for dev = 0, MAX_NUMBER_OF_DEVICES - 1 do
                local address       = ct:key_read(config.arch.stm32f1.key["I2C"..I2C.."_DEV_"..dev.."_ADDRESS"]):gsub("0x", "")
                local addr10bit     = ct:yes_no_to_bool(ct:key_read(config.arch.stm32f1.key["I2C"..I2C.."_DEV_"..dev.."_10BIT_ADDR"]))
                local sub_addr_mode = tonumber(ct:key_read(config.arch.stm32f1.key["I2C"..I2C.."_DEV_"..dev.."_SEND_SUB_ADDR"]))

                ui.Panel_device[dev]:Enable(dev < no_of_dev)
                ui.TextCtrl_address[dev]:SetValue(address)
                ui.Choice_addr_mode[dev]:SetSelection(ifs(addr10bit, 1, 0))
                ui.Choice_subaddr_mode[dev]:SetSelection(sub_addr_mode)
        end

        ui.Panel_peripheral:Enable(I2C_enable)
end


--------------------------------------------------------------------------------
-- @brief  Event is called when Save button is clicked
-- @param  None
-- @return None
--------------------------------------------------------------------------------
local function on_button_save_click()
        local I2C = ui.Choice_I2C:GetSelection() + 1


        ct:enable_module("I2C", ui.CheckBox_enable:GetValue())


        local I2C_enable = ct:bool_to_yes_no(ui.CheckBox_I2C_enable:IsChecked())
        ct:key_write(config.arch.stm32f1.key["I2C"..I2C.."_ENABLE"], I2C_enable)


        local use_DMA = ct:bool_to_yes_no(ui.CheckBox_use_DMA:IsChecked())
        ct:key_write(config.arch.stm32f1.key["I2C"..I2C.."_USE_DMA"], use_DMA)


        local IRQ_prio = ui.Choice_IRQ_priority:GetSelection() + 1
        if IRQ_prio > #prio_list then
                IRQ_prio = config.project.def.DEFAULT_IRQ_PRIORITY:GetValue()
        else
                IRQ_prio = prio_list[IRQ_prio].value
        end
        ct:key_write(config.arch.stm32f1.key["I2C"..I2C.."_IRQ_PRIO"], IRQ_prio)


        local SCL_freq = tostring(ui.SpinCtrl_SCL_freq:GetValue() * 1000)
        ct:key_write(config.arch.stm32f1.key["I2C"..I2C.."_SCL_FREQ"], SCL_freq)


        local no_of_dev = tostring(ui.Choice_dev_no:GetSelection() + 1)
        ct:key_write(config.arch.stm32f1.key["I2C"..I2C.."_NUMBER_OF_DEVICES"], no_of_dev)


        for dev = 0, MAX_NUMBER_OF_DEVICES - 1 do
                local address       = "0x"..ui.TextCtrl_address[dev]:GetValue()
                local addr10bit     = ct:bool_to_yes_no(ui.Choice_addr_mode[dev]:GetSelection() > 0)
                local sub_addr_mode = tostring(ui.Choice_subaddr_mode[dev]:GetSelection())

                ct:key_write(config.arch.stm32f1.key["I2C"..I2C.."_DEV_"..dev.."_ADDRESS"], address)
                ct:key_write(config.arch.stm32f1.key["I2C"..I2C.."_DEV_"..dev.."_10BIT_ADDR"], addr10bit)
                ct:key_write(config.arch.stm32f1.key["I2C"..I2C.."_DEV_"..dev.."_SEND_SUB_ADDR"], sub_addr_mode)
        end


        ui.Button_save:Enable(false)
end


--------------------------------------------------------------------------------
-- @brief  Event is called when module enable checkbox is changed
-- @param  this         event object
-- @return None
--------------------------------------------------------------------------------
local function checkbox_enable_updated(this)
        ui.Panel_module:Enable(this:IsChecked())
        ui.Button_save:Enable(true)
end


--------------------------------------------------------------------------------
-- @brief  Event is called when value is updated
-- @param  None
-- @return None
--------------------------------------------------------------------------------
local function event_value_updated()
        ui.Button_save:Enable(true)
end


--------------------------------------------------------------------------------
-- @brief  Event is called when I2C device is enabled
-- @param  None
-- @return None
--------------------------------------------------------------------------------
local function event_checkbox_I2C_enable(this)
        ui.Panel_peripheral:Enable(this:IsChecked())
        ui.Button_save:Enable(true)
end


--------------------------------------------------------------------------------
-- @brief  Event is called when number of devices is updated
-- @param  None
-- @return None
--------------------------------------------------------------------------------
local function event_number_of_devices_updated(this)
        local dev_number = this:GetSelection()

        for i = 0, MAX_NUMBER_OF_DEVICES - 1 do
                ui.Panel_device[i]:Enable(i <= dev_number)
        end

        ui.Button_save:Enable(true)
end


--------------------------------------------------------------------------------
-- @brief  Event is called when I2C peripheral is selected
-- @param  None
-- @return None
--------------------------------------------------------------------------------
local function event_I2C_selected()
        if ui.Choice_I2C.OldSelection ~= ui.Choice_I2C:GetSelection() then

                local answer = wx.wxID_NO
                if ui.Button_save:IsEnabled() then
                        answer = ct:show_question_msg(ct.MAIN_WINDOW_NAME, "Do you want to SAVE changes?", bit.bor(wx.wxYES_NO, wx.wxCANCEL))
                end

                if answer == wx.wxID_YES then
                        local to_save = ui.Choice_I2C.OldSelection
                        ui.Choice_I2C.OldSelection = ui.Choice_I2C:GetSelection()
                        ui.Choice_I2C:SetSelection(to_save)

                        on_button_save_click()
                        ui.Choice_I2C:SetSelection(ui.Choice_I2C.OldSelection)

                elseif answer == wx.wxID_NO then
                        ui.Choice_I2C.OldSelection = ui.Choice_I2C:GetSelection()

                elseif answer == wx.wxID_CANCEL then
                        ui.Choice_I2C:SetSelection(ui.Choice_I2C.OldSelection)
                        return
                end

                load_controls()

                if answer == wx.wxID_YES or answer == wx.wxID_NO then
                        ui.Button_save:Enable(false)
                end
        end
end


--==============================================================================
-- GLOBAL FUNCTIONS
--==============================================================================
--------------------------------------------------------------------------------
-- @brief  Function creates a new window
-- @param  parent       parent window
-- @return New window handle
--------------------------------------------------------------------------------
function i2c:create_window(parent)
        cpu_name = ct:key_read(config.arch.stm32f1.key.CPU_NAME)
        cpu_idx  = ct:get_cpu_index("stm32f1", cpu_name)
        i2c_cfg  = config.arch.stm32f1.cpulist:Children()[cpu_idx].peripherals.I2C

        ui = {}
        ui.Panel_device = {}
        ui.TextCtrl_address = {}
        ui.Choice_addr_mode = {}
        ui.Choice_subaddr_mode = {}

        ID = {}
        ID.CHECKBOX_ENABLE = wx.wxNewId()
        ID.PANEL_MODULE = wx.wxNewId()
        ID.CHOICE_I2C = wx.wxNewId()
        ID.CHECKBOX_I2C_ENABLE = wx.wxNewId()
        ID.PANEL_PERIPHERAL = wx.wxNewId()
        ID.CHECKBOX_USE_DMA = wx.wxNewId()
        ID.CHOICE_IRQ_PRIORITY = wx.wxNewId()
        ID.SPINCTRL_SCL_FREQ = wx.wxNewId()
        ID.CHOICE_DEV_NO = wx.wxNewId()
        ID.PANEL_DEVICE = {}
        ID.TEXTCTRL_ADDRESS = {}
        ID.CHOICE_ADDR_MODE = {}
        ID.CHOICE_SUBADDR_MODE = {}
        ID.BUTTON_SAVE = wx.wxNewId()

        ui.window  = wx.wxScrolledWindow(parent, wx.wxID_ANY)
        local this = ui.window

        ui.FlexGridSizer1 = wx.wxFlexGridSizer(0, 1, 0, 0)
        ui.CheckBox_enable = wx.wxCheckBox(this, ID.CHECKBOX_ENABLE, "Enable module", wx.wxDefaultPosition, wx.wxDefaultSize)
        ui.FlexGridSizer1:Add(ui.CheckBox_enable, 1, bit.bor(wx.wxALL,wx.wxALIGN_LEFT,wx.wxALIGN_CENTER_VERTICAL), 5)

        ui.Panel_module = wx.wxPanel(this, ID.PANEL_MODULE, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxTAB_TRAVERSAL)
        ui.FlexGridSizer_module = wx.wxFlexGridSizer(0, 1, 0, 0)

        ui.Choice_I2C = wx.wxChoice(ui.Panel_module, ID.CHOICE_I2C, wx.wxDefaultPosition, wx.wxSize(ct.CONTROL_X_SIZE, -1))
        for i = 1, i2c_cfg:NumChildren() do
                ui.Choice_I2C:Append("I2C"..i2c_cfg:Children()[i].name:GetValue())
        end
        ui.Choice_I2C:SetSelection(0)
        ui.Choice_I2C.OldSelection = 0
        ui.FlexGridSizer_module:Add(ui.Choice_I2C, 1, bit.bor(wx.wxALL,wx.wxEXPAND,wx.wxALIGN_CENTER_HORIZONTAL,wx.wxALIGN_CENTER_VERTICAL), 5)

        ui.CheckBox_I2C_enable = wx.wxCheckBox(ui.Panel_module, ID.CHECKBOX_I2C_ENABLE, "Enable peripheral", wx.wxDefaultPosition, wx.wxDefaultSize)
        ui.FlexGridSizer_module:Add(ui.CheckBox_I2C_enable, 1, bit.bor(wx.wxALL,wx.wxALIGN_LEFT,wx.wxALIGN_CENTER_VERTICAL), 5)

        ui.Panel_peripheral = wx.wxPanel(ui.Panel_module, ID.PANEL_PERIPHERAL, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxTAB_TRAVERSAL)
        ui.FlexGridSizer_peripheral = wx.wxFlexGridSizer(0, 1, 0, 0)
        ui.FlexGridSizer_settings = wx.wxFlexGridSizer(0, 2, 0, 0)

        ui.CheckBox_use_DMA = wx.wxCheckBox(ui.Panel_peripheral, ID.CHECKBOX_USE_DMA, "Use DMA", wx.wxDefaultPosition, wx.wxDefaultSize)
        ui.FlexGridSizer_settings:Add(ui.CheckBox_use_DMA, 1, bit.bor(wx.wxALL,wx.wxALIGN_LEFT,wx.wxALIGN_CENTER_VERTICAL), 0)
        ui.StaticText5 = wx.wxStaticText(ui.Panel_peripheral, wx.wxID_ANY, "", wx.wxDefaultPosition, wx.wxDefaultSize)
        ui.StaticText5:Hide()
        ui.FlexGridSizer_settings:Add(ui.StaticText5, 1, bit.bor(wx.wxALL,wx.wxALIGN_CENTER_HORIZONTAL,wx.wxALIGN_CENTER_VERTICAL), 5)

        ui.StaticText2 = wx.wxStaticText(ui.Panel_peripheral, wx.wxID_ANY, "IRQ priority", wx.wxDefaultPosition, wx.wxDefaultSize)
        ui.FlexGridSizer_settings:Add(ui.StaticText2, 1, bit.bor(wx.wxALL,wx.wxALIGN_LEFT,wx.wxALIGN_CENTER_VERTICAL), 0)
        ui.Choice_IRQ_priority = wx.wxChoice(ui.Panel_peripheral, ID.CHOICE_IRQ_PRIORITY, wx.wxDefaultPosition, wx.wxDefaultSize, {}, 0)
        for i, item in ipairs(prio_list) do
                ui.Choice_IRQ_priority:Append(item.name)
        end
        ui.Choice_IRQ_priority:Append("System default")
        ui.FlexGridSizer_settings:Add(ui.Choice_IRQ_priority, 1, bit.bor(wx.wxALL,wx.wxEXPAND,wx.wxALIGN_LEFT,wx.wxALIGN_CENTER_VERTICAL), 5)

        ui.StaticText3 = wx.wxStaticText(ui.Panel_peripheral, wx.wxID_ANY, "SCL frequency [kHz]", wx.wxDefaultPosition, wx.wxDefaultSize)
        ui.FlexGridSizer_settings:Add(ui.StaticText3, 1, bit.bor(wx.wxALL,wx.wxALIGN_LEFT,wx.wxALIGN_CENTER_VERTICAL), 0)
        ui.SpinCtrl_SCL_freq = wx.wxSpinCtrl(ui.Panel_peripheral, ID.SPINCTRL_SCL_FREQ, "0", wx.wxDefaultPosition, wx.wxDefaultSize, 0, 10, 400, 0)
        ui.FlexGridSizer_settings:Add(ui.SpinCtrl_SCL_freq, 1, bit.bor(wx.wxALL,wx.wxEXPAND,wx.wxALIGN_LEFT,wx.wxALIGN_CENTER_VERTICAL), 5)

        ui.StaticText1 = wx.wxStaticText(ui.Panel_peripheral, wx.wxID_ANY, "Number of devices", wx.wxDefaultPosition, wx.wxDefaultSize)
        ui.FlexGridSizer_settings:Add(ui.StaticText1, 1, bit.bor(wx.wxALL,wx.wxALIGN_LEFT,wx.wxALIGN_CENTER_VERTICAL), 0)
        ui.Choice_dev_no = wx.wxChoice(ui.Panel_peripheral, ID.CHOICE_DEV_NO, wx.wxDefaultPosition, wx.wxDefaultSize, {}, 0)
        ui.Choice_dev_no:Append({"1", "2", "3", "4", "5", "6", "7", "8"})
        ui.FlexGridSizer_settings:Add(ui.Choice_dev_no, 1, bit.bor(wx.wxALL,wx.wxEXPAND,wx.wxALIGN_LEFT,wx.wxALIGN_CENTER_VERTICAL), 5)

        ui.FlexGridSizer_peripheral:Add(ui.FlexGridSizer_settings, 0, bit.bor(wx.wxALIGN_LEFT,wx.wxALIGN_TOP), 0)

        for dev = 0, MAX_NUMBER_OF_DEVICES - 1 do
                ID.PANEL_DEVICE[dev] = wx.wxNewId()
                ui.Panel_device[dev] = wx.wxPanel(ui.Panel_peripheral, ID.PANEL_DEVICE[dev], wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxTAB_TRAVERSAL)
                ui.FlexGridSizer_device = wx.wxFlexGridSizer(0, 4, 0, 0)
                ui.StaticText_device = wx.wxStaticText(ui.Panel_device[dev], wx.wxID_ANY, "Device "..dev..":  0x", wx.wxDefaultPosition, wx.wxDefaultSize)
                ui.FlexGridSizer_device:Add(ui.StaticText_device, 1, bit.bor(wx.wxALL,wx.wxALIGN_RIGHT,wx.wxALIGN_CENTER_VERTICAL), 0)

                ID.TEXTCTRL_ADDRESS[dev] = wx.wxNewId()
                ui.TextCtrl_address[dev] = wx.wxTextCtrl(ui.Panel_device[dev], ID.TEXTCTRL_ADDRESS[dev], "", wx.wxDefaultPosition, wx.wxDefaultSize, 0, ct.hexvalidator)
                ui.TextCtrl_address[dev]:SetMinSize(wx.wxSize(50,-1))
                ui.TextCtrl_address[dev]:SetMaxSize(wx.wxSize(50,-1))
                ui.TextCtrl_address[dev]:SetMaxLength(3)
                ui.FlexGridSizer_device:Add(ui.TextCtrl_address[dev], 1, bit.bor(wx.wxALL,wx.wxALIGN_RIGHT,wx.wxALIGN_CENTER_VERTICAL), 0)

                ID.CHOICE_ADDR_MODE[dev] = wx.wxNewId()
                ui.Choice_addr_mode[dev] = wx.wxChoice(ui.Panel_device[dev], ID.CHOICE_ADDR_MODE[dev], wx.wxDefaultPosition, wx.wxDefaultSize, {}, 0)
                ui.Choice_addr_mode[dev]:Append("7-bit address")
                ui.Choice_addr_mode[dev]:Append("10-bit address")
                ui.FlexGridSizer_device:Add(ui.Choice_addr_mode[dev], 1, bit.bor(wx.wxALL,wx.wxEXPAND,wx.wxALIGN_LEFT,wx.wxALIGN_CENTER_VERTICAL), 5)

                ID.CHOICE_SUBADDR_MODE[dev] = wx.wxNewId()
                ui.Choice_subaddr_mode[dev] = wx.wxChoice(ui.Panel_device[dev], ID.CHOICE_SUBADDR_MODE[dev], wx.wxDefaultPosition, wx.wxDefaultSize, {}, 0)
                ui.Choice_subaddr_mode[dev]:Append("No sub-address")
                ui.Choice_subaddr_mode[dev]:Append("1 byte sub-address")
                ui.Choice_subaddr_mode[dev]:Append("2 bytes sub-address")
                ui.Choice_subaddr_mode[dev]:Append("3 bytes sub-address")
                ui.FlexGridSizer_device:Add(ui.Choice_subaddr_mode[dev], 1, bit.bor(wx.wxALL,wx.wxEXPAND,wx.wxALIGN_CENTER_HORIZONTAL,wx.wxALIGN_CENTER_VERTICAL), 5)

                ui.Panel_device[dev]:SetSizer(ui.FlexGridSizer_device)
                ui.FlexGridSizer_device:Fit(ui.Panel_device[dev])
                ui.FlexGridSizer_device:SetSizeHints(ui.Panel_device[dev])
                ui.FlexGridSizer_peripheral:Add(ui.Panel_device[dev], 1, bit.bor(wx.wxALL,wx.wxEXPAND,wx.wxALIGN_CENTER_HORIZONTAL,wx.wxALIGN_CENTER_VERTICAL), 0)

                this:Connect(ID.TEXTCTRL_ADDRESS[dev],    wx.wxEVT_COMMAND_TEXT_UPDATED,    event_value_updated)
                this:Connect(ID.CHOICE_ADDR_MODE[dev],    wx.wxEVT_COMMAND_CHOICE_SELECTED, event_value_updated)
                this:Connect(ID.CHOICE_SUBADDR_MODE[dev], wx.wxEVT_COMMAND_CHOICE_SELECTED, event_value_updated)
        end


        ui.Panel_peripheral:SetSizer(ui.FlexGridSizer_peripheral)
        ui.FlexGridSizer_module:Add(ui.Panel_peripheral, 1, bit.bor(wx.wxALL,wx.wxEXPAND,wx.wxALIGN_CENTER_HORIZONTAL,wx.wxALIGN_CENTER_VERTICAL), 5)

        ui.Panel_module:SetSizer(ui.FlexGridSizer_module)
        ui.FlexGridSizer1:Add(ui.Panel_module, 1, bit.bor(wx.wxALL,wx.wxEXPAND,wx.wxALIGN_CENTER_HORIZONTAL,wx.wxALIGN_CENTER_VERTICAL), 5)

        ui.StaticLine1 = wx.wxStaticLine(this, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxSize(10,-1), wx.wxLI_HORIZONTAL)
        ui.FlexGridSizer1:Add(ui.StaticLine1, 1, bit.bor(wx.wxALL,wx.wxEXPAND,wx.wxALIGN_CENTER_HORIZONTAL,wx.wxALIGN_CENTER_VERTICAL), 5)

        ui.Button_save = wx.wxButton(this, ID.BUTTON_SAVE, "Save", wx.wxDefaultPosition, wx.wxDefaultSize)
        ui.FlexGridSizer1:Add(ui.Button_save, 1, bit.bor(wx.wxALL,wx.wxALIGN_RIGHT,wx.wxALIGN_CENTER_VERTICAL), 5)

        --
        this:SetSizer(ui.FlexGridSizer1)
        this:SetScrollRate(12, 12)

        --
        this:Connect(ID.CHECKBOX_ENABLE, wx.wxEVT_COMMAND_CHECKBOX_CLICKED, checkbox_enable_updated)
        this:Connect(ID.BUTTON_SAVE,     wx.wxEVT_COMMAND_BUTTON_CLICKED,   on_button_save_click   )
        this:Connect(ID.CHECKBOX_I2C_ENABLE, wx.wxEVT_COMMAND_CHECKBOX_CLICKED, event_checkbox_I2C_enable)
        this:Connect(ID.CHECKBOX_USE_DMA, wx.wxEVT_COMMAND_CHECKBOX_CLICKED, event_value_updated)
        this:Connect(ID.CHOICE_DEV_NO, wx.wxEVT_COMMAND_CHOICE_SELECTED, event_number_of_devices_updated)
        this:Connect(ID.CHOICE_IRQ_PRIORITY, wx.wxEVT_COMMAND_CHOICE_SELECTED, event_value_updated)
        this:Connect(ID.SPINCTRL_SCL_FREQ, wx.wxEVT_COMMAND_SPINCTRL_UPDATED, event_value_updated)
        this:Connect(ID.CHOICE_I2C, wx.wxEVT_COMMAND_CHOICE_SELECTED, event_I2C_selected)

        --
        load_controls()
        ui.Button_save:Enable(false)

        return ui.window
end


--------------------------------------------------------------------------------
-- @brief  Function returns module name
-- @param  None
-- @return Module name
--------------------------------------------------------------------------------
function i2c:get_window_name()
        return "I2C"
end


--------------------------------------------------------------------------------
-- @brief  Function is called by parent when window is selected
-- @param  None
-- @return None
--------------------------------------------------------------------------------
function i2c:selected()
end


--------------------------------------------------------------------------------
-- @brief  Function returns modify status
-- @param  None
-- @return If data is modified true is returned, otherwise false
--------------------------------------------------------------------------------
function i2c:is_modified()
        return ui.Button_save:IsEnabled()
end


--------------------------------------------------------------------------------
-- @brief  Function returns module handler
-- @param  None
-- @return Module handler
--------------------------------------------------------------------------------
function get_handler()
        return i2c
end
