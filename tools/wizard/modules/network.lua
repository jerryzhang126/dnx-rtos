--[[============================================================================
@file    network.lua

@author  Daniel Zorychta

@brief   Network configuration file.

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


*//*========================================================================--]]

require "defs"
require "db"

--------------------------------------------------------------------------------
-- GLOBAL OBJECTS
--------------------------------------------------------------------------------
-- public calls objects
net = {}

--------------------------------------------------------------------------------
-- FUNCTIONS
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
-- @brief Calculate total steps of this configuration
--------------------------------------------------------------------------------
local function calculate_total_steps()
        if (key_read(db.path.project.flags, "__NETWORK_ENABLE__") == yes) then
                set_total_steps(3)
        else
                set_total_steps(1)
        end
end

--------------------------------------------------------------------------------
-- @brief Function validate MAC address
--------------------------------------------------------------------------------
local function is_MAC_valid(mac_str)
        if (mac_str:match("\"([a-fA-F0-9][a-fA-F0-9]:[a-fA-F0-9][a-fA-F0-9]:[a-fA-F0-9][a-fA-F0-9]:"..
                             "[a-fA-F0-9][a-fA-F0-9]:[a-fA-F0-9][a-fA-F0-9]:[a-fA-F0-9][a-fA-F0-9])\"") ) then

                return true
        else
                return false
        end
end

--------------------------------------------------------------------------------
-- @brief Function validate MAC address
--------------------------------------------------------------------------------
local function get_MAC_elements(mac_str)
        return mac_str:match("\"([a-fA-F0-9][a-fA-F0-9]):([a-fA-F0-9][a-fA-F0-9]):([a-fA-F0-9][a-fA-F0-9]):"..
                               "([a-fA-F0-9][a-fA-F0-9]):([a-fA-F0-9][a-fA-F0-9]):([a-fA-F0-9][a-fA-F0-9])\"")
end

--------------------------------------------------------------------------------
-- @brief Configure net enable
--------------------------------------------------------------------------------
local function configure_net_enable()
        local choice = key_read(db.path.project.flags, "__NETWORK_ENABLE__")
        msg(progress() .. "Do you want to enable network?")
        msg("Current choice is: " .. string.gsub(choice, "_", "") .. ".")
        add_item(yes, "YES")
        add_item(no, "NO")
        choice = get_selection()
        if (can_be_saved(choice)) then
                key_save(db.path.project.flags, "__NETWORK_ENABLE__", choice)
                key_save(db.path.project.mk,    "ENABLE_NETWORK",     choice)
        end

        calculate_total_steps()

        return choice
end

--------------------------------------------------------------------------------
-- @brief Configure MAC address
--------------------------------------------------------------------------------
local function configure_mac()
        local MAC = {}
        MAC[0] = key_read(db.path.project.flags, "__NETWORK_MAC_ADDR_0__")
        MAC[1] = key_read(db.path.project.flags, "__NETWORK_MAC_ADDR_1__")
        MAC[2] = key_read(db.path.project.flags, "__NETWORK_MAC_ADDR_2__")
        MAC[3] = key_read(db.path.project.flags, "__NETWORK_MAC_ADDR_3__")
        MAC[4] = key_read(db.path.project.flags, "__NETWORK_MAC_ADDR_4__")
        MAC[5] = key_read(db.path.project.flags, "__NETWORK_MAC_ADDR_5__")

        for i = 0, 5 do
                MAC[i] = string.gsub(MAC[i], "0x", "")
        end

        local first_time = true
        local correct    = false

        while not correct do
                msg(progress() .. "MAC address configuration. Enter MAC address in format: xx:xx:xx:xx:xx:xx.")
                msg("Current MAC address is: " .. MAC[0] .. ":" .. MAC[1] .. ":" ..  MAC[2] .. ":" ..  MAC[3] .. ":" ..  MAC[4] .. ":" ..  MAC[5] .. ".")
                if not first_time then msg("You have entered an invalid address, try again.") end
                str = get_string()
                if (can_be_saved(str)) then
                        if is_MAC_valid(str) then
                                local a, b, c, d, e, f = get_MAC_elements(str)

                                key_save(db.path.project.flags, "__NETWORK_MAC_ADDR_0__", "0x"..a:upper())
                                key_save(db.path.project.flags, "__NETWORK_MAC_ADDR_1__", "0x"..b:upper())
                                key_save(db.path.project.flags, "__NETWORK_MAC_ADDR_2__", "0x"..c:upper())
                                key_save(db.path.project.flags, "__NETWORK_MAC_ADDR_3__", "0x"..d:upper())
                                key_save(db.path.project.flags, "__NETWORK_MAC_ADDR_4__", "0x"..e:upper())
                                key_save(db.path.project.flags, "__NETWORK_MAC_ADDR_5__", "0x"..f:upper())

                                correct = true
                        else
                                modify_current_step(-1)
                        end
                else
                        correct = true
                end

                first_time = false
        end

        return str
end

--------------------------------------------------------------------------------
-- @brief Configure Ethernet file
--------------------------------------------------------------------------------
local function configure_eth_file()
        local str = key_read(db.path.project.flags, "__NETWORK_ETHIF_FILE__")
        msg(progress() .. "Enter Ethernet file path.")
        msg("Current file is: " .. str .. ".")
        str = get_string()
        if (can_be_saved(str)) then
                key_save(db.path.project.flags, "__NETWORK_ETHIF_FILE__", str)
        end

        return str
end

--------------------------------------------------------------------------------
-- @brief Configuration summary
--------------------------------------------------------------------------------
local function print_summary()
        local net_en = key_read(db.path.project.flags, "__NETWORK_ENABLE__")
        local file   = key_read(db.path.project.flags, "__NETWORK_ETHIF_FILE__")
        local MAC    = {}
        MAC[0]       = key_read(db.path.project.flags, "__NETWORK_MAC_ADDR_0__"):gsub("0x", "")
        MAC[1]       = key_read(db.path.project.flags, "__NETWORK_MAC_ADDR_1__"):gsub("0x", "")
        MAC[2]       = key_read(db.path.project.flags, "__NETWORK_MAC_ADDR_2__"):gsub("0x", "")
        MAC[3]       = key_read(db.path.project.flags, "__NETWORK_MAC_ADDR_3__"):gsub("0x", "")
        MAC[4]       = key_read(db.path.project.flags, "__NETWORK_MAC_ADDR_4__"):gsub("0x", "")
        MAC[5]       = key_read(db.path.project.flags, "__NETWORK_MAC_ADDR_5__"):gsub("0x", "")

        msg("Network configuration summary:")
        msg("Enabled: "..filter_yes_no(net_en).."\n"..
            "MAC: "..MAC[0]..":"..MAC[1]..":"..MAC[2]..":"..MAC[3]..":"..MAC[4]..":"..MAC[5].."\n"..
            "Ethernet interface file: "..file)

        pause()
end

--------------------------------------------------------------------------------
-- @brief Function execute configuration
--------------------------------------------------------------------------------
function net:configure()
        title("Network Configuration")
        navigation("Home/Network")
        calculate_total_steps()

        ::net_enable:: if configure_net_enable() == back then return back end

        if (key_read(db.path.project.flags, "__NETWORK_ENABLE__") == no) then
                msg("To configure more network options, enable network.")
                pause()
                return next
        end

        ::mac_cfg::  if configure_mac()      == back then goto net_enable end
        ::eth_file:: if configure_eth_file() == back then goto mac_cfg    end

        print_summary()

        return next
end

-- started without master file
if (master ~= true) then
        show_no_master_info()
end

--------------------------------------------------------------------------------
-- END OF FILE
--------------------------------------------------------------------------------