--[[============================================================================
@file    project.lua

@author  Daniel Zorychta

@brief   Project configuration file.

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

--------------------------------------------------------------------------------
-- FUNCTIONS
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
-- @brief Function configure project name
--------------------------------------------------------------------------------
local function configure_project_name()
        msg(progress() .. "Project name configuration.")
        local str = key_read("../project/Makefile", "PROJECT_NAME")
        msg("Current project name is: " .. str .. ".")
        str = get_string()
        if (can_be_saved(str)) then
                key_save("../project/Makefile", "PROJECT_NAME", str)
        end

        return str
end

--------------------------------------------------------------------------------
-- @brief Function configure toolchain name
--------------------------------------------------------------------------------
local function configure_toolchain_name()
        local message = "Toolchain name configuration."
        local curr_is = "Current toolchain name is: "
        local choice  = key_read("../project/Makefile", "PROJECT_TOOLCHAIN")

        msg(progress() .. message)
        msg(curr_is .. choice)
        add_item("arm-none-eabi-", "arm-none-eabi - e.g. Linaro, CodeSourcery")
        add_item("other", "Other")
        local name = get_selection()
        if (can_be_saved(name)) then
                if (name == "other") then
                        modify_current_step(-1)
                        msg(progress() .. message)
                        msg(curr_is .. choice)
                        name = get_string()
                end

                if (can_be_saved(name)) then
                        key_save("../project/Makefile", "PROJECT_TOOLCHAIN", name)
                end
        end

        return name
end

--------------------------------------------------------------------------------
-- @brief Function execute configuration
--------------------------------------------------------------------------------
function project_configure()
        set_total_steps(2)

        title("General project configuration")

        ::_1_::
        if configure_project_name() == back then
                return back
        end

        ::_2_::
        if configure_toolchain_name() == back then
                goto _1_
        end

        return next
end

--------------------------------------------------------------------------------
-- Enable configuration if master wizard is not defined
--------------------------------------------------------------------------------
if (master ~= true) then
        while project_configure() == back do
        end

        configuration_finished()
end

--------------------------------------------------------------------------------
-- END OF FILE
--------------------------------------------------------------------------------
