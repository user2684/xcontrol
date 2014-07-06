#####################################
######X-Control Enhanced Plugin######
#####################################
This multi-platform X-Plane plugin provides enhanced controls for the flight simulator. It supports Saitek x52 and x52 Pro joysticks allowing to customize additional data pages on the Multi-Functional Display (MFD) with up to four configurable buttons. 
Additionally, it comes with an embedded realistic FMS capable of calculating expected enroute time (considering realistic climb/descend), required fuel to load, top of climb/ descend distance, time and waypoint. 
The FMS continuously monitors your position and leverages X-Plane voice to brief you regarding the flight plan status, position report and delays from scheduled and expected time.
Even without a supported joystick, both the FMS and a virtual MFD are made available.
 
###### Supported Platforms ######
X-Plane 9.x and 10.x:
- Mac OSX (32 bit and 64 bit)
- Linux (32 bit and 64 bit)
- Windows (32 bit and 64 bit)

###### FMS Features ######
Once a valid flight plan is loaded in the FMS and the plugin is configured with scheduled departure time, cruise ground speed and altitude, it will provide the following information:
	- Expected Enroute Time (considering realistic climb/descend time)
	- Total Distance
	- Required fuel to load
	- Top of Climb/ Top of Descend distance and time
	- For each entry: leg time, leg duration, distance from departure, distance from arrival, scheduled time, expected altitude
Additionally, the FMS continuously monitors your position and keeps track of the following information:
	- Time at departure/arrival gate
	- For each entry actual time/altitude and delay from scheduled and expected time
The plugin also leverages X-Plane voice to brief you regarding the flight plan status and position report.

###### Installation ######
Mac OSX:
- uncompress the package
- Copy the entire "xcontrol" directory into "x-plane_directory/Resources/plugins"
Linux:
- uncompress the package
- Copy the entire "xcontrol" directory into "x-plane_directory/Resources/plugins"
- As root, copy the "90-saitekX52.rules" file from the plugin direction into "/etc/udev/rules.d" 
- As root, run "/etc/init.d/udev reload"
Windows:
- uncompress the package
- Copy the entire "xcontrol" directory into "x-plane_directory/Resources/plugins"
- Download and install Visual C++ Redistributable Packages for Visual Studio 2013 from http://www.microsoft.com/en-us/download/details.aspx?id#40784
- If you have a Saitek joystick, go on with the following instructions to install the required libraries
- Connect your joystick
- Download and uncormpress the libusb-win32-filter.zip package available from the plugin download page
- If on a 64 bit system, run "libusb-win32-filter\64\install-filter-win.exe", if on a 32 bit system run "libusb-win32-filter\64\install-filter-win.exe"
- Select "Install a device Filter" and click "Next"
- Select your joystick (for Saitek, it is the one with vid:06a3) and click on "Install"
- Open the start menu, run "Test (Win) program" under "Programs\LibUSB-Win32" and ensure you can see the model of your joystick
	
###### Upgrade ######
If upgrading from x52control:
- uncompress the package
- Copy the entire "xcontrol" directory into "x-plane_directory/Resources/plugins"
- Copy the "x52control.ini" and "x52fms.ini" files from "x-plane_directory/Resources/plugins/x52control" into "x-plane_directory/Resources/plugins/xcontrol" and rename them into "xcontrol.ini" and "xfms.ini"
- Delete the existing x52control installation by moving the directory "x-plane_directory/Resources/plugins/x52control" away from the plugin directory
If NOT upgrading from x52control:
- uncompress the package
- Copy and overwrite the directories "32" and "64" from the upgrade package into "x-plane_directory/Resources/plugins/xcontrol" to maintain the existing configuration settings

###### Basic Configuration ######
- Optionally edit the xcontrol.ini file in the plugin directory if you want to customize the MFD pages to display (see below "xcontrol.ini configuration")
- Optionally edit the xfms.ini file in the plugin directory if you want to customize the FMS settings (see below "xfms.ini configuration")
- Connect the Joystick and run X-Plane
- Configure some buttons (on the joystick or not) to make the MFD cycling between different pages. Go to "Joystick & Equipment", choose the "Buttons: Adv" tab, press the  button you want to use for switching between different MFD pages, select "instruments" from the box in the middle and "xcontrol_cycle_button_1" from the box on the right
- Optionally do the same for "xcontrol_cycle_button_2", "xcontrol_cycle_button_3" , "xcontrol_cycle_button_4" 
- Click the configured buttons to see the MFD changing. Click the same button more then once to cycle among the pages.
- Load or configure a valid flight plan
- Set scheduled departure time, cruise ground speed and altitude in the menu "Plugin", "XControl", "FMS Settings"
- While flying, track your progress along the plan by leveraging the MFD or with the summary available in the menu "Plugin", "XControl", "Show FMS Status" (accessible also with the hotkey CTRL+Shift+B)
- The same information printed on the MFD is also available on the virtual MFD available in the menu "Plugin", "XControl", "Show virtual MFD" (accessible also with the hotkey CTRL+Shift+V)
- Please note that xcontrol is delivered as a "fat" plugin; this means that the correct plugin for your platform/architecture is automatically loaded by X-Plane. However, old X-Plane 9.x versions do not support this feature; so if the plugin is not loading, manually copy the correct xpl file out from the "32" or "64" directory into the plugin root directory.

###### xcontrol.ini configuration ######
The xcontrol.ini file can be used to customize buttons, leds and MFD pages. Open the file with a text editor; instructions for each directive are provided within the file. Default values should be fine. 
If you wish to customize for each button the MFD pages to display and cycle within, add the name of the MFD template to the directives mfdpages_button1, mfdpages_button2, mfdpages_button3, mfdpages_button4 (e.g. mfdpages_button1#flight.fms,flight.combined).
For a complete list of the available MFD data pages, see below ("Available Data Pages").

###### xfms.ini configuration ######
The xms.ini file can be used to customize FMS settings used by the plugin to calculate times and distances. Open the file with a text editor; instructions for each directive are provided within the file.
Planned cruise Flight Level, Ground Speed and expected takeoff time can be configured from the plugin menu and there is no need to change the file for those. Climbing/descending profiles are used to calculate top of climb/descend. Each profile contains the time and distance for different Flight Levels (e.g. how many minutes/miles to climb to the specific altitude/descending from the specific altitude). 
Fuel consumption on ground (taxi,takeoff,landing), for climbing/descending, cruise consumption for 100nm, IFR/VFR contingency can all be configure within this file.
Since some of those values can be aircraft specific, the plugin looks also for a xfms.ini file into the loaded aircraft folder, if found, it takes precedence over the global file. You may need to reload the plugin to have this to work.
	
##### Available Data Pages ######
A list of all the available data pages follows below. For each page the template is also provided to give an idea of what is going to show up in the screen when selected

Flight control related pages:

- flight.combined:
	S:[IAS] M:[match_speed] T:[trasponder]  
	FL:[FL] V:[vs]00 [gear]
	[nav1_id] [nav1_dme]nm F:[flap]
- flight.low
    [IAS] R:[rain_perc]-[runway_friction] V:[visibility] [trasponder]
	FL:[FL] V:[vs] F:[flap]
	[cur_nav_id] [nav_dme] [wind_dir]/[wind_speed]
- flight.high:
	[IAS] [TAS]/[GS] [mach]
	FL:[FL] T:[temp] W:[wind_speed]
	[cur_nav_id] [nav_dme]nm [hh]:[mm]

FMS related pages:
- fms.status (next entries):
	[arrival_apt] [remaining_distance] [fuel_eta]
	[hh_eta]:[mm_eta] [id_name]
	[distance]/[remaining]m [hh_time]:[mm_time]
- fms.status (previous entries):	
	[id_name] [time_actual]
	[time_expected] ([delay_from_scheduled]/[delay_from_expected])
	[altitude_expected] ([altitude_actual])
- fms.planner:
	[departure_apt]->[arrival_apt] [total_distance]
	D:[EET] F:[required_fuel_lb]
	TD:[miles] to [FIX_top_of_descend]
- fms.scheduler:
	T:[expected_takeoff_time] ([actual_takeoff_time])
	C:[expected_top_of_climb] D:[expected_top_of_desc]
	T:[expected_landing_time] ([actual_landing_time])

Weather related pages: 
- weather.wind:
    W:F[fl1] [wind_dir]/[wind_speed]:[turbulence]T
	W:F[fl2] [wind_dir]/[wind_speed]:[turbulence]T
	W:F[fl3] [wind_dir]/[wind_speed]:[turbulence]T
- weather.cloud:
	C:F[fl1_base]-[fl1_top] [clouds]
	C:F[fl2_base]-[fl2_top] [clouds]
	C:F[fl3_base]-[fl3_top] [clouds]
- weather.condition:
	T:[ground_temp]C [wing_temp]C [outside_temp]C
	W:[ground_wind_dir]/[ground_wind_speed] [outside_wind_dir]/[outside_wind_speed]
	V:[visibility]k R:[rain_level] RWY:[rwy_friction] 

Legacy pages:
	- std.radio.nav
	- std.radio.com
	- std.radio.adf
	- std.navigation
	
###### Source code ######
The source code is available on GitHub at the following location: https://github.com/user2684/xcontrol

###### Credits ######
This plugin has been derived from x52control 0.4.0-dt whose source code is available at https://github.com/sothis/x52control/tree/v0.4.0-dt
