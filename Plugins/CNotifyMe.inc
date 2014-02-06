/*  Copyright 2014 MorpheusX(AUT)

    https://myrcon.com

    This file is part of MorpheusX(AUT)'s Plugins for BFBC2 PRoCon.

    MorpheusX(AUT)'s Plugins for BFBC2 PRoCon is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MorpheusX(AUT)'s Plugins for BFBC2 PRoCon is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BFBC2 PRoCon.  If not, see <http://www.gnu.org/licenses/>.
 */

using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Mail;
using System.Text;
using System.Threading;
using System.Security.Cryptography;

using PRoCon.Core;
using PRoCon.Core.Plugin;
using PRoCon.Core.Plugin.Commands;
using PRoCon.Core.Players;
using PRoCon.Core.Players.Items;
using PRoCon.Core.Battlemap;
using PRoCon.Core.Maps;

namespace PRoConEvents
{
    public class CNotifyMe : PRoConPluginAPI, IPRoConPluginInterface
    {
        #region variables + constructor

        private bool blPluginEnabled;
        private string strHostName;
        private string strPort;
        private CServerInfo csiServer;

        private enumBoolYesNo blNotifyConsole;
        private enumBoolYesNo blNotifyChat;
        private enumBoolYesNo blNotifyNotification;
        private enumBoolYesNo blNotifyIngame;
        private string strIngameNotification;
        private enumBoolYesNo blNotifyEmail;
		private string strGUIDType;
        private enumBoolYesNo blMonitorChat;
        private List<string> lstKeywordList;
        private string strPreviousMessage;
        private enumBoolYesNo blSpamProtection;
        private int iAllowedRequests;
        private int iTimeBetweenRequests;
        private string strGameType;

        private enumBoolYesNo blUseProconAccounts;
        private enumBoolYesNo blUseReservedSlots;
        private enumBoolYesNo blUseCustomList;

        private string strAdminsCommand;
        private string strCallAdminCommand;
		private enumBoolYesNo blRequireReason;

        private List<CPlayerInfo> lstPlayers;
        private List<string> lstReservedSlots;
        private List<string> lstCustomAdmins;

        private enumBoolYesNo blAlwaysSendMail;
        private enumBoolYesNo blUseSSL;
        private string strSMTPServer;
        private int iSMTPPort;
        private string strSenderMail;
        private List<string> lstReceiverMail;
        private string strSMTPUser;
        private string strSMTPPassword;

        private Dictionary<string, CSpamProtection> dicAntiSpam;
		private Dictionary<string, CPunkbusterInfo> dicPBList;

        public CNotifyMe()
        {
            this.blPluginEnabled = false;

            this.blNotifyConsole = enumBoolYesNo.Yes;
            this.blNotifyChat = enumBoolYesNo.No;
            this.blNotifyNotification = enumBoolYesNo.No;
            this.blNotifyIngame = enumBoolYesNo.No;
            this.strIngameNotification = "Say";
            this.blNotifyEmail = enumBoolYesNo.No;
			this.strGUIDType = "Both";
            this.blMonitorChat = enumBoolYesNo.No;
            this.lstKeywordList = new List<string>();
            this.strPreviousMessage = String.Empty;
            this.blSpamProtection = enumBoolYesNo.Yes;
            this.iAllowedRequests = 5;
            this.iTimeBetweenRequests = 60;
            this.strGameType = "BF4";

            this.blUseProconAccounts = enumBoolYesNo.Yes;
            this.blUseReservedSlots = enumBoolYesNo.No;
            this.blUseCustomList = enumBoolYesNo.No;

            this.strAdminsCommand = "admins";
            this.strCallAdminCommand = "calladmin";
			this.blRequireReason = enumBoolYesNo.No;

            this.lstPlayers = new List<CPlayerInfo>();
            this.lstReservedSlots = new List<string>();
            this.lstCustomAdmins = new List<string>();

            this.blAlwaysSendMail = enumBoolYesNo.No;
            this.blUseSSL = enumBoolYesNo.No;
            this.strSMTPServer = String.Empty;
            this.iSMTPPort = 25;
            this.strSenderMail = String.Empty;
            this.lstReceiverMail = new List<string>();
            this.strSMTPUser = String.Empty;
            this.strSMTPPassword = String.Empty;

            this.dicAntiSpam = new Dictionary<string, CSpamProtection>();
			this.dicPBList = new Dictionary<string, CPunkbusterInfo>();
        }

        #endregion

        #region plugin details

        // sets the name displayed in Procon's plugin-tab
        public string GetPluginName()
        {
            return "Notify Me!";
        }

        // plugin-version shown in the plugin-tab
        public string GetPluginVersion()
        {
            return "1.0.0.3";
        }

        // your name
        public string GetPluginAuthor()
        {
            return "MorpheusX(AUT)";
        }

        // url of your homepage
        public string GetPluginWebsite()
        {
            return "https://forum.myrcon.com/showthread.php?2294-Notify-Me!-Notifications-and-alerts-(v1-0-0-2)";
        }

        // description displayed in the description-tab of the plugins
        // use HTML-tags to layout the text
        public string GetPluginDescription()
        {
            return @"
<p>This Plugin was written by MorpheusX(AUT).<br />
<b>Twitter:</b> <a href='http://twitter.com/#!/MorpheusXAUT'>MorpheusXAUT</a><br />
<b>myrcon.com:</b> <a href='https://forum.myrcon.com/member.php?565-MorpheusX(AUT)'>MorpheusX(AUT)</a><br />

<h2>Description</h2>
<p><b>Notify Me!</b> is a new attempt to combine several notification plugins currently available. The plugin provides two new features to the players on your server:<br /></p>
<ol><li><p><b>Listing admins currently playing</b>: when using this command, the player gets a list of admins currently online on the server. In additions, the admins get notified that someone has requested this list.</p></li>
<li><p><b>Calling an admin</b>: every player can call for an admin. Depending on the plugin's settings, admins get notified ingame, in Procon and via mail. An optional reason for calling can be provided by the user.</p></li></ol><br />
<p>The plugin is compatible with Procon's ingame help-system, the two commands can also be found typing '@help' into ingame chat.</p>

<h2>Plugin setup</h2>
<p>All plugin-settings will be explained in the following paragraphs:<br /></p>
<h3>1. Notification Settings</h3>
<ul><li><p><b>Notify in Plugin Console?</b>: post a message into Procon's plugin-console when an relevant event is caught</p></li>
<li><p><b>Notify in Chat Window?</b>: post a message into Procon's chat-window when an relevant event is caught</p></li>
<li><p><b>Notify in SysTray Popup?</b>: post a message into SysTray Popup when an relevant event is caught. This is obsolete when using a layer. A layer-client-popup system is in development</p></li>
<li><p><b>Notify admins ingame?</b>: post a message to all admins currently ingame when an relevant event is caught</p></li>
<li><p><b>Ingame notification type</b>: way of notifying ingame admins</p></li>
<li><p><b>Notify via email?</b>: send an email to defined addresses when an relevant event is caught</p></li>
<li><p><b>GUID type shown in email</b>: selects which GUID type will be shown in the report email</p></li>
<li><p><b>Monitor chat for keywords?</b>: check all chat-entries for defined buzzwords</p></li>
<li><p><b>Keyword list</b>: list of buzzwords for chat-monitoring. This list is case-insentive and also matches partially (e.g.: 'cheat' triggers 'cheater' too)</p></li>
<li><p><b>Enable spam protection?</b>: toggle spam protection to prevent alert-spam</p></li>
<li><p><b>Number of allowed requests per round</b>: number of allowed admin-calls/admin-lists per round. Gets reset for every player at the end of a round</p></li>
<li><p><b>Time between requests (seconds)</b>: time in seconds, which must elapse before a player can send a new request (set to 0 to disable)</p></li></ul>
<h3>2. Admin Settings</h3>
<ul><li><p><b>Use Procon-Accounts as admins?</b>: the plugin will check whether a player has ingame-kick rights. If that's the case, he will be listed as an admin</p></li>
<li><p><b>Use reserved slotlist as admins?</b>: the plugin will check whether a player is listed on the gameserver's reserved slotlist. If that's the case, he will be listed as an admin</p></li>
<li><p><b>Use custom adminlist as admins?</b>: the plugin will check whether a player is listed on a custom list defined in the plugin settings. If that's the case, he will be listed as an admin</p></li>
<li><p><b>Custom adminlist</b>: list containing all custom admins defined by the headadmin. This list is case-insensitive</p></li></ul>
<h3>3. Ingame Commands</h3>
<ul><li><p><b>List admins on the server</b>: command used to list all admins online on the server. Don't include '@' in this string. All commands are available using '@', '!' or '#' as a prefix. The current command is '@" + this.strAdminsCommand + @"'</p></li>
<li><p><b>Call an admin</b>: command used to call an admin. Don't include '@' in this string. All commands are available using '@', '!' or '#' as a prefix. The current command is '@" + this.strCallAdminCommand + @"'</p></li>
<li><p><b>Require reason for calling an admin?</b>: Toggles whether players <b>must</b> provide a reason when calling for an admins. If this is set to 'Yes', requests with no reasons will be ignored and players will be notified.</p></li></ul>
<h3>4. Email Settings</h3>
<ul><li><p><b>Always send email notifications?</b>: when turned on, notification-mails will be sent every time a player requests an admin. when turned off, the mails will just be sent when no admin is currently ingame</p></li>
<li><p><b>Use SSL?</b>: toggle SSL usage for mail-transmission</p></li>
<li><p><b>SMTP-Server address</b>: hostname or IP of the SMTP-server used for sending mails</p></li>
<li><p><b>SMTP-Server port</b>: SMTP-port used to connect to the server. Default is 25 (not 110 like I tried while testing, duh...)</p></li>
<li><p><b>Sender address</b>: email-address used as a sender for the notification-mails</p></li>
<li><p><b>Receiver addresses</b>: list of addresses, to which the notification-mails will be sent</p></li>
<li><p><b>SMTP-Server username</b>: username used to authenticate at the SMTP-server</p></li>
<li><p><b>SMTP-Server password</b>: password used to authenticate at the SMTP-server</p></li></ul>
";
        }

        // called every time the plugin is loaded (on start and when clicking "reload plugins")
        public void OnPluginLoaded(string strHostName, string strPort, string strPRoConVersion)
        {
            this.strHostName = strHostName;
            this.strPort = strPort;
        }
		
		public void OnPluginLoadingEnv(List<string> lstPluginEnv)
		{
			this.strGameType = lstPluginEnv[1];
		}

        // called when the plugin gets enabled. This also includes loading a plugin, which has been enabled before
        public void OnPluginEnable()
        {
            this.blPluginEnabled = true;

            this.RegisterEvents(this.GetType().Name, "OnListPlayers", "OnPlayerJoin", "OnGlobalChat", "OnTeamChat", "OnSquadChat", "OnServerInfo", "OnPlayerLeft", "OnReservedSlotsList", "OnRoundOver", "OnLoadingLevel", "OnAccountLogin", "OnAccountLogout", "OnPunkbusterPlayerInfo");

            this.RegisterAllCommands();

            this.PrepareAdmins();

            this.ConsoleWrite("^b^2Enabled =)^0");
        }

        // called when the plugin gets disabled. Doesn't get called when loading a disabled plugin
        public void OnPluginDisable()
        {
            this.blPluginEnabled = false;

			this.dicAntiSpam.Clear();
			this.dicPBList.Clear();
			
            this.UnregisterAllCommands();

            this.ConsoleWrite("^b^1Disabled :(^0");
        }

        // setting up the plugin's variables displayed to the user
        public List<CPluginVariable> GetDisplayPluginVariables()
        {
            List<CPluginVariable> lstReturn = new List<CPluginVariable>();

            lstReturn.Add(new CPluginVariable("2. Admin Settings|Use Procon-Accounts as admins?", typeof(enumBoolYesNo), this.blUseProconAccounts));
            lstReturn.Add(new CPluginVariable("2. Admin Settings|Use reserved slotlist as admins?", typeof(enumBoolYesNo), this.blUseReservedSlots));
            lstReturn.Add(new CPluginVariable("2. Admin Settings|Use custom adminlist as admins?", typeof(enumBoolYesNo), this.blUseCustomList));
            if (this.blUseCustomList == enumBoolYesNo.Yes)
            {
                lstReturn.Add(new CPluginVariable("2. Admin Settings|Custom adminlist", typeof(string[]), this.lstCustomAdmins.ToArray()));
            }

            lstReturn.Add(new CPluginVariable("1. Notification Settings|Notify in Plugin Console?", typeof(enumBoolYesNo), this.blNotifyConsole));
            lstReturn.Add(new CPluginVariable("1. Notification Settings|Notify in Chat Window?", typeof(enumBoolYesNo), this.blNotifyChat));
            lstReturn.Add(new CPluginVariable("1. Notification Settings|Notify in SysTray Popup?", typeof(enumBoolYesNo), this.blNotifyNotification));
            lstReturn.Add(new CPluginVariable("1. Notification Settings|Notify admins ingame?", typeof(enumBoolYesNo), this.blNotifyIngame));
            if (this.blNotifyIngame == enumBoolYesNo.Yes)
            {
                lstReturn.Add(new CPluginVariable("1. Notification Settings|Ingame notification type", "enumActions(Say|Yell|Both)", this.strIngameNotification));
            }
            lstReturn.Add(new CPluginVariable("1. Notification Settings|Notify via email?", typeof(enumBoolYesNo), this.blNotifyEmail));
			if (this.blNotifyEmail == enumBoolYesNo.Yes)
			{
				lstReturn.Add(new CPluginVariable("1. Notification Settings|GUID type shown in email", "enumGUIDType(EA GUID|PB GUID|Both)", this.strGUIDType));
			}
            lstReturn.Add(new CPluginVariable("1. Notification Settings|Monitor chat for keywords?", typeof(enumBoolYesNo), this.blMonitorChat));
            if (this.blMonitorChat == enumBoolYesNo.Yes)
            {
                lstReturn.Add(new CPluginVariable("1. Notification Settings|Keyword list", typeof(string[]), this.lstKeywordList.ToArray()));
            }
            lstReturn.Add(new CPluginVariable("1. Notification Settings|Enable spam protection?", typeof(enumBoolYesNo), this.blSpamProtection));
            if (this.blSpamProtection == enumBoolYesNo.Yes)
            {
                lstReturn.Add(new CPluginVariable("1. Notification Settings|Number of allowed requests per round", typeof(int), this.iAllowedRequests));
                lstReturn.Add(new CPluginVariable("1. Notification Settings|Time between requests (seconds)", typeof(int), this.iTimeBetweenRequests));
            }

            lstReturn.Add(new CPluginVariable("3. Ingame Commands|List admins on the server", typeof(string), this.strAdminsCommand));
            lstReturn.Add(new CPluginVariable("3. Ingame Commands|Call an admin", typeof(string), this.strCallAdminCommand));
			lstReturn.Add(new CPluginVariable("3. Ingame Commands|Require reason for calling an admin?", typeof(enumBoolYesNo), this.blRequireReason));

            if (this.blNotifyEmail == enumBoolYesNo.Yes)
            {
                lstReturn.Add(new CPluginVariable("4. Email Settings|Always send email notifications?", typeof(enumBoolYesNo), this.blAlwaysSendMail));
                lstReturn.Add(new CPluginVariable("4. Email Settings|Use SSL?", typeof(enumBoolYesNo), this.blUseSSL));
                lstReturn.Add(new CPluginVariable("4. Email Settings|SMTP-Server address", typeof(string), this.strSMTPServer));
                lstReturn.Add(new CPluginVariable("4. Email Settings|SMTP-Server port", typeof(int), this.iSMTPPort));
                lstReturn.Add(new CPluginVariable("4. Email Settings|Sender address", typeof(string), this.strSenderMail));
                lstReturn.Add(new CPluginVariable("4. Email Settings|Receiver addresses", typeof(string[]), this.lstReceiverMail.ToArray()));
                lstReturn.Add(new CPluginVariable("4. Email Settings|SMTP-Server username", typeof(string), this.strSMTPUser));
                lstReturn.Add(new CPluginVariable("4. Email Settings|SMTP-Server password", typeof(string), this.strSMTPPassword));
            }

            return lstReturn;
        }

        // setting up the plugin's variables as they are saved
        public List<CPluginVariable> GetPluginVariables()
        {
            List<CPluginVariable> lstReturn = new List<CPluginVariable>();

            lstReturn.Add(new CPluginVariable("Use Procon-Accounts as admins?", typeof(enumBoolYesNo), this.blUseProconAccounts));
            lstReturn.Add(new CPluginVariable("Use reserved slotlist as admins?", typeof(enumBoolYesNo), this.blUseReservedSlots));
            lstReturn.Add(new CPluginVariable("Use custom adminlist as admins?", typeof(enumBoolYesNo), this.blUseCustomList));
            lstReturn.Add(new CPluginVariable("Custom adminlist", typeof(string[]), this.lstCustomAdmins.ToArray()));

            lstReturn.Add(new CPluginVariable("Notify in Plugin Console?", typeof(enumBoolYesNo), this.blNotifyConsole));
            lstReturn.Add(new CPluginVariable("Notify in Chat Window?", typeof(enumBoolYesNo), this.blNotifyChat));
            lstReturn.Add(new CPluginVariable("Notify in SysTray Popup?", typeof(enumBoolYesNo), this.blNotifyNotification));
            lstReturn.Add(new CPluginVariable("Notify admins ingame?", typeof(enumBoolYesNo), this.blNotifyIngame));
            lstReturn.Add(new CPluginVariable("Ingame notification type", "enumActions(Say|Yell|Both)", this.strIngameNotification));
            lstReturn.Add(new CPluginVariable("Notify via email?", typeof(enumBoolYesNo), this.blNotifyEmail));
			lstReturn.Add(new CPluginVariable("GUID type shown in email", "enumGUIDType(EA GUID|PB GUID|Both)", this.strGUIDType));
            lstReturn.Add(new CPluginVariable("Monitor chat for keywords?", typeof(enumBoolYesNo), this.blMonitorChat));
            lstReturn.Add(new CPluginVariable("Keyword list", typeof(string[]), this.lstKeywordList.ToArray()));
            lstReturn.Add(new CPluginVariable("Enable spam protection?", typeof(enumBoolYesNo), this.blSpamProtection));
            lstReturn.Add(new CPluginVariable("Number of allowed requests per round", typeof(int), this.iAllowedRequests));
            lstReturn.Add(new CPluginVariable("Time between requests (seconds)", typeof(int), this.iTimeBetweenRequests));

            lstReturn.Add(new CPluginVariable("List admins on the server", typeof(string), this.strAdminsCommand));
            lstReturn.Add(new CPluginVariable("Call an admin", typeof(string), this.strCallAdminCommand));
			lstReturn.Add(new CPluginVariable("Require reason for calling an admin?", typeof(enumBoolYesNo), this.blRequireReason));

            lstReturn.Add(new CPluginVariable("Always send email notifications?", typeof(enumBoolYesNo), this.blAlwaysSendMail));
            lstReturn.Add(new CPluginVariable("Use SSL?", typeof(enumBoolYesNo), this.blUseSSL));
            lstReturn.Add(new CPluginVariable("SMTP-Server address", typeof(string), this.strSMTPServer));
            lstReturn.Add(new CPluginVariable("SMTP-Server port", typeof(int), this.iSMTPPort));
            lstReturn.Add(new CPluginVariable("Sender address", typeof(string), this.strSenderMail));
            lstReturn.Add(new CPluginVariable("Receiver addresses", typeof(string[]), this.lstReceiverMail.ToArray()));
            lstReturn.Add(new CPluginVariable("SMTP-Server username", typeof(string), this.strSMTPUser));
            lstReturn.Add(new CPluginVariable("SMTP-Server password", typeof(string), this.strSMTPPassword));
            
            return lstReturn;
        }

        // called when a plugin-variable is changed by the admin
        public void SetPluginVariable(string strVariable, string strValue)
        {
            this.UnregisterAllCommands();

            int iPort = 0;
            int iRequests = 0;
            int iTime = 0;

            if (strVariable.CompareTo("Notify in Plugin Console?") == 0 && Enum.IsDefined(typeof(enumBoolYesNo), strValue) == true)
            {
                this.blNotifyConsole = (enumBoolYesNo)Enum.Parse(typeof(enumBoolYesNo), strValue);
            }
            else if (strVariable.CompareTo("Notify in Chat Window?") == 0 && Enum.IsDefined(typeof(enumBoolYesNo), strValue) == true)
            {
                this.blNotifyChat = (enumBoolYesNo)Enum.Parse(typeof(enumBoolYesNo), strValue);
            }
            else if (strVariable.CompareTo("Notify in SysTray Popup?") == 0 && Enum.IsDefined(typeof(enumBoolYesNo), strValue) == true)
            {
                this.blNotifyNotification = (enumBoolYesNo)Enum.Parse(typeof(enumBoolYesNo), strValue);
            }
            else if (strVariable.CompareTo("Notify admins ingame?") == 0 && Enum.IsDefined(typeof(enumBoolYesNo), strValue) == true)
            {
                this.blNotifyIngame = (enumBoolYesNo)Enum.Parse(typeof(enumBoolYesNo), strValue);

                if (this.blNotifyIngame == enumBoolYesNo.Yes && (this.blUseProconAccounts == enumBoolYesNo.No && this.blUseReservedSlots == enumBoolYesNo.No && this.blUseCustomList == enumBoolYesNo.No))
                {
                    this.ConsoleWrite("You haven't set any list for ingame admins!");
                }
            }
            else if (strVariable.CompareTo("Ingame notification type") == 0)
            {
                this.strIngameNotification = strValue;
            }
            else if (strVariable.CompareTo("Notify via email?") == 0 && Enum.IsDefined(typeof(enumBoolYesNo), strValue) == true)
            {
                this.blNotifyEmail = (enumBoolYesNo)Enum.Parse(typeof(enumBoolYesNo), strValue);
            }
			else if (strVariable.CompareTo("GUID type shown in email") == 0)
            {
                this.strGUIDType = strValue;
            }
            else if (strVariable.CompareTo("Monitor chat for keywords?") == 0 && Enum.IsDefined(typeof(enumBoolYesNo), strValue) == true)
            {
                this.blMonitorChat = (enumBoolYesNo)Enum.Parse(typeof(enumBoolYesNo), strValue);
            }
            else if (strVariable.CompareTo("Keyword list") == 0)
            {
                this.lstKeywordList = new List<string>(CPluginVariable.DecodeStringArray(strValue));
            }
            else if (strVariable.CompareTo("Enable spam protection?") == 0 && Enum.IsDefined(typeof(enumBoolYesNo), strValue) == true)
            {
                this.blSpamProtection = (enumBoolYesNo)Enum.Parse(typeof(enumBoolYesNo), strValue);
            }
            else if (strVariable.CompareTo("Number of allowed requests per round") == 0 && int.TryParse(strValue, out iRequests) == true)
            {
                if (iRequests > 0)
                {
                    this.iAllowedRequests = iRequests;
                }
            }
            else if (strVariable.CompareTo("Time between requests (seconds)") == 0 && int.TryParse(strValue, out iTime) == true)
            {
                if (iTime >= 0)
                {
                    this.iTimeBetweenRequests = iTime;
                }
            }
            else if (strVariable.CompareTo("Use Procon-Accounts as admins?") == 0 && Enum.IsDefined(typeof(enumBoolYesNo), strValue) == true)
            {
                this.blUseProconAccounts = (enumBoolYesNo)Enum.Parse(typeof(enumBoolYesNo), strValue);

                if (this.blUseProconAccounts == enumBoolYesNo.Yes)
                {
                    this.PrepareAdmins();
                }
            }
            else if (strVariable.CompareTo("Use reserved slotlist as admins?") == 0 && Enum.IsDefined(typeof(enumBoolYesNo), strValue) == true)
            {
                this.blUseReservedSlots = (enumBoolYesNo)Enum.Parse(typeof(enumBoolYesNo), strValue);

                if (this.blUseReservedSlots == enumBoolYesNo.Yes)
                {
                    this.PrepareAdmins();
                }
            }
            else if (strVariable.CompareTo("Use custom adminlist as admins?") == 0 && Enum.IsDefined(typeof(enumBoolYesNo), strValue) == true)
            {
                this.blUseCustomList = (enumBoolYesNo)Enum.Parse(typeof(enumBoolYesNo), strValue);

                if (this.blUseCustomList == enumBoolYesNo.Yes)
                {
                    this.PrepareAdmins();
                }
            }
            else if (strVariable.CompareTo("Custom adminlist") == 0)
            {
                this.lstCustomAdmins = new List<string>(CPluginVariable.DecodeStringArray(strValue));

                this.PrepareAdmins();
            }
            else if (strVariable.CompareTo("List admins on the server") == 0)
            {
                this.strAdminsCommand = strValue;
            }
            else if (strVariable.CompareTo("Call an admin") == 0)
            {
                this.strCallAdminCommand = strValue;
            }
			else if (strVariable.CompareTo("Require reason for calling an admin?") == 0 && Enum.IsDefined(typeof(enumBoolYesNo), strValue) == true)
            {
                this.blRequireReason = (enumBoolYesNo)Enum.Parse(typeof(enumBoolYesNo), strValue);
            }
            else if (strVariable.CompareTo("Always send email notifications?") == 0 && Enum.IsDefined(typeof(enumBoolYesNo), strValue) == true)
            {
                this.blAlwaysSendMail = (enumBoolYesNo)Enum.Parse(typeof(enumBoolYesNo), strValue);
            }
            else if (strVariable.CompareTo("Use SSL?") == 0 && Enum.IsDefined(typeof(enumBoolYesNo), strValue) == true)
            {
                this.blUseSSL = (enumBoolYesNo)Enum.Parse(typeof(enumBoolYesNo), strValue);
            }
            else if (strVariable.CompareTo("SMTP-Server address") == 0)
            {
                this.strSMTPServer = strValue;
            }
            else if (strVariable.CompareTo("SMTP-Server port") == 0 && int.TryParse(strValue, out iPort) == true)
            {
                if (iPort > 0)
                {
                    this.iSMTPPort = iPort;
                }
            }
            else if (strVariable.CompareTo("Sender address") == 0)
            {
                this.strSenderMail = strValue;
            }
            else if (strVariable.CompareTo("Receiver addresses") == 0)
            {
                this.lstReceiverMail = new List<string>(CPluginVariable.DecodeStringArray(strValue));
            }
            else if (strVariable.CompareTo("SMTP-Server username") == 0)
            {
                this.strSMTPUser = strValue;
            }
            else if (strVariable.CompareTo("SMTP-Server password") == 0)
            {
                this.strSMTPPassword = strValue;
            }

            this.RegisterAllCommands();
        }

        #endregion

        #region own methods

        private void RegisterAllCommands()
        {
            if (this.blPluginEnabled)
            {
                List<string> emptyList = new List<string>();

                this.RegisterCommand(
                    new MatchCommand(
                        "CNotifyMe",
                        "OnCommandAdmins",
                        this.Listify<string>("@", "!", "#"),
                        this.strAdminsCommand,
                        this.Listify<MatchArgumentFormat>(),
                        new ExecutionRequirements(
                            ExecutionScope.All),
                        "Lists all admins currently online on this server"
                    )
                );

                this.RegisterCommand(
                    new MatchCommand(
                        "CNotifyMe",
                        "OnCommandCallAdmin",
                        this.Listify<string>("@", "!", "#"),
                        this.strCallAdminCommand,
                        this.Listify<MatchArgumentFormat>(
                            new MatchArgumentFormat(
                                "reason",
                                emptyList
                            )
                        ),
                        new ExecutionRequirements(
                            ExecutionScope.All),
                        "Call for an admin with an optional reason"
                    )
                );
            }
        }

        private void UnregisterAllCommands()
        {
            List<string> emptyList = new List<string>();

            this.UnregisterCommand(
                new MatchCommand(
                    "CNotifyMe",
                    "OnCommandAdmins",
                    this.Listify<string>("@", "!", "#"),
                    this.strAdminsCommand,
                    this.Listify<MatchArgumentFormat>(),
                    new ExecutionRequirements(
                        ExecutionScope.All
                        ),
                    "Lists all admins currently online on this server"
                )
            );

            this.UnregisterCommand(
                new MatchCommand(
                    "CNotifyMe",
                    "OnCommandCallAdmin",
                    this.Listify<string>("@", "!", "#"),
                    this.strCallAdminCommand,
                    this.Listify<MatchArgumentFormat>(
                        new MatchArgumentFormat(
                            "reason",
                            emptyList
                        )
                    ),
                    new ExecutionRequirements(
                        ExecutionScope.All
                        ),
                    "Lists all admins currently online on this server"
                )
            );
        }

        private void PrepareAdmins()
        {
            this.ExecuteCommand("procon.protected.send", "reservedSlots.list");
            Thread.Sleep(100);
            this.ExecuteCommand("procon.protected.send", "admin.listPlayers", "all");
        }

        private void ConsoleWrite(string message)
        {
            this.ExecuteCommand("procon.protected.pluginconsole.write", String.Format("^b^1Notify Me!^0^n: {0}", message));
        }

        private void ChatWrite(string message)
        {
            this.ExecuteCommand("procon.protected.chat.write", String.Format("^b^1Notify Me!^0^n: {0}", message));
        }

        private void NotificationWrite(string message, string blImportant)
        {
            this.ExecuteCommand("procon.protected.notification.write", "Notify Me!", message, blImportant);
        }

        private void IngameWrite(string message)
        {
			List<string> adminsOnline = this.AdminsOnline();
		
            foreach (string str in adminsOnline)
            {
                if (this.strIngameNotification.CompareTo("Say") == 0)
                {
                    this.ExecuteCommand("procon.protected.send", "admin.say", "[Notify Me!] " +  message, "player", str);
                }
                else if (this.strIngameNotification.CompareTo("Yell") == 0)
                {
                    if (this.strGameType.CompareTo("BC2") == 0)
                    {
                        this.ExecuteCommand("procon.protected.send", "admin.yell", "[Notify Me!] " + message, "8000", "player", str);
                    }
                    else if (this.strGameType.CompareTo("BF3") == 0 || this.strGameType.CompareTo("BF4") == 0)
                    {
                        this.ExecuteCommand("procon.protected.send", "admin.yell", "[Notify Me!] " + message, "8", "player", str);
                    }
					else
					{
						this.ExecuteCommand("procon.protected.send", "admin.say", "[Notify Me!] " + message, "player", str);
					}
                }
                else if (this.strIngameNotification.CompareTo("Both") == 0)
                {
                    this.ExecuteCommand("procon.protected.send", "admin.say", "[Notify Me!] " + message, "player", str);
                    if (this.strGameType.CompareTo("BC2") == 0)
                    {
                        this.ExecuteCommand("procon.protected.send", "admin.yell", "[Notify Me!] " + message, "8000", "player", str);
                    }
                    else if (this.strGameType.CompareTo("BF3") == 0 || this.strGameType.CompareTo("BF4") == 0)
                    {
                        this.ExecuteCommand("procon.protected.send", "admin.yell", "[Notify Me!] " + message, "8", "player", str);
                    }
                }
            }
        }

        private void PrepareEmail(string sender, string message)
        {
            if (this.blNotifyEmail == enumBoolYesNo.Yes)
            {
                string subject = String.Empty;
                string body = String.Empty;

                subject = "[Procon Notify Me!] Admin Request - " + this.csiServer.ServerName;

                StringBuilder sb = new StringBuilder();
                sb.Append("<b>Notify Me! for Procon - CallForAdmin Notification</b><br /><br />");
                sb.Append("<i>Date/Time of call:</i> " + DateTime.Now.ToString() + "<br />");
                sb.Append("<i>Servername:</i> " + this.csiServer.ServerName + "<br />");
                sb.Append("<i>Server address:</i> " + this.strHostName + ":" + this.strPort + "<br />");
                sb.Append("<i>Playercount:</i> " + this.csiServer.PlayerCount + "/" + this.csiServer.MaxPlayerCount + "<br />");
                sb.Append("<i>Map:</i> " + this.csiServer.Map + "<br /><br />");
                sb.Append("<i>Request-Sender:</i> " + sender + "<br />");
                sb.Append("<i>Message:</i> " + message + "<br /><br />");
                sb.Append("<i>Playertable:</i><br />");
                sb.Append("<table border='1' ><tr><th>ClanTag</th><th>Playername</th>");
				
				if (this.strGUIDType.CompareTo("EA GUID") == 0)
				{
					sb.Append("<th>EA GUID</th>");
				}
				else if (this.strGUIDType.CompareTo("PB GUID") == 0)
				{
					sb.Append("<th>PB GUID</th>");
				}
				else
				{
					sb.Append("<th>EA GUID</th><th>PB GUID</th>");
				}
				
				sb.Append("<th>IP</th><th>Score</th><th>Kills</th><th>Deaths</th><th>KDR</th><th>Ping</th><th>Rank</th></tr>");
				
                foreach (CPlayerInfo player in this.lstPlayers)
                {
                    sb.AppendFormat("<tr><td>{0}</td><td>{1}", player.ClanTag, player.SoldierName); 
					if (this.strGUIDType.CompareTo("EA GUID") == 0)
					{
						sb.AppendFormat("<td>{0}</td>", player.GUID);
					}
					else if (this.strGUIDType.CompareTo("PB GUID") == 0)
					{
						sb.AppendFormat("<td>{0}</td>", this.dicPBList.ContainsKey(player.SoldierName) ? this.dicPBList[player.SoldierName].GUID : String.Empty);
					}
					else
					{
						sb.AppendFormat("<td>{0}</td><td>{1}</td>", player.GUID, this.dicPBList.ContainsKey(player.SoldierName) ? this.dicPBList[player.SoldierName].GUID : String.Empty);
					}
					sb.AppendFormat("<td>{0}</td><td>{1}</td><td>{2}</td><td>{3}</td><td>{4}</td><td>{5}</td><td>{6}</td></tr>", this.dicPBList.ContainsKey(player.SoldierName) ? this.dicPBList[player.SoldierName].Ip : String.Empty, player.Score, player.Kills, player.Deaths, Math.Round((decimal)player.Kdr, 2), player.Ping, player.Rank);
                }
                sb.Append("</table>");

                body = sb.ToString();

                this.EmailWrite(subject, body);
            }
        }

        private void EmailWrite(string subject, string body)
        {
            try
            {
                if (this.strSenderMail == null || this.strSenderMail == String.Empty)
                {
                    this.ConsoleWrite("No sender-mail is given!");
                    return;
                }

                MailMessage email = new MailMessage();

                email.From = new MailAddress(this.strSenderMail);

                if (this.lstReceiverMail.Count > 0)
                {
                    foreach (string mailto in this.lstReceiverMail)
                    {
                        if (mailto.Contains("@") && mailto.Contains("."))
                        {
                            email.To.Add(new MailAddress(mailto));
                        }
                        else
                        {
                            this.ConsoleWrite("Error in receiver-mail: " + mailto);
                        }
                    }
                }
                else
                {
                    this.ConsoleWrite("No receiver-mail are given!");
                    return;
                }

                email.Subject = subject;
                email.Body = body;
                email.IsBodyHtml = true;
                email.BodyEncoding = UTF8Encoding.UTF8;

                SmtpClient smtp = new SmtpClient(this.strSMTPServer, this.iSMTPPort);
                if (this.blUseSSL == enumBoolYesNo.Yes)
                {
                    smtp.EnableSsl = true;
                }
                else if (this.blUseSSL == enumBoolYesNo.No)
                {
                    smtp.EnableSsl = false;
                }
                smtp.Timeout = 10000;
                smtp.DeliveryMethod = SmtpDeliveryMethod.Network;
                smtp.UseDefaultCredentials = false;
                smtp.Credentials = new NetworkCredential(this.strSMTPUser, this.strSMTPPassword);
                smtp.Send(email);

                this.ConsoleWrite("A notification email has been sent.");
            }
            catch (Exception e)
            {
                this.ConsoleWrite("Error while sending mails: " + e.ToString());
            }
        }

        private bool SpamCheck(string name)
        {
            if (this.blSpamProtection == enumBoolYesNo.Yes)
            {
                if (!this.dicAntiSpam.ContainsKey(name))
                {
                    this.dicAntiSpam.Add(name, new CSpamProtection(name));
                }

                if (this.dicAntiSpam[name].iRequestCount >= this.iAllowedRequests)
                {
                    this.ExecuteCommand("procon.protected.send", "admin.say", String.Format("[Notify Me!] You have reached the allowed numbers of requests per round ({0})!", this.iAllowedRequests), "player", name);
                    return true;
                }

                TimeSpan tsTimeSinceLastRequest = DateTime.Now.Subtract(this.dicAntiSpam[name].dtLastRequest);
                if (tsTimeSinceLastRequest.TotalSeconds <= this.iTimeBetweenRequests && this.dicAntiSpam[name].iRequestCount != 0)
                {
                    this.ExecuteCommand("procon.protected.send", "admin.say", String.Format("[Notify Me!] You can just send a request every {0} seconds ({1} seconds since last request, {2}/{3} requests this round)!", this.iTimeBetweenRequests, (int)tsTimeSinceLastRequest.TotalSeconds, this.dicAntiSpam[name].iRequestCount, this.iAllowedRequests), "player", name);
                    return true;
                }

                this.dicAntiSpam[name].iRequestCount++;
                this.dicAntiSpam[name].dtLastRequest = DateTime.Now;
            }

            return false;
        }

		public List<string> AdminsOnline()
		{
			List<string> adminsOnline = new List<string>();
		
			foreach (CPlayerInfo cpiPlayer in this.lstPlayers)
			{
				if (this.blUseProconAccounts == enumBoolYesNo.Yes)
				{
					CPrivileges privileges = this.GetAccountPrivileges(cpiPlayer.SoldierName);
					if (privileges != null && privileges.CanKickPlayers)
					{
						adminsOnline.Add(cpiPlayer.SoldierName);
					}
				}

				if (this.blUseReservedSlots == enumBoolYesNo.Yes)
				{
					foreach (string str in this.lstReservedSlots)
					{
						if (cpiPlayer.SoldierName.CompareTo(str) == 0 && (adminsOnline.Contains(cpiPlayer.SoldierName) == false))
						{
							adminsOnline.Add(cpiPlayer.SoldierName);
							break;
						}
					}
				}

				if (this.blUseCustomList == enumBoolYesNo.Yes)
				{
					foreach (string str in this.lstCustomAdmins)
					{
						if (cpiPlayer.SoldierName.ToLower().CompareTo(str.ToLower()) == 0 && (adminsOnline.Contains(cpiPlayer.SoldierName) == false))
						{
							adminsOnline.Add(cpiPlayer.SoldierName);
							break;
						}
					}
				}

				if (!this.dicAntiSpam.ContainsKey(cpiPlayer.SoldierName))
				{
					this.dicAntiSpam.Add(cpiPlayer.SoldierName, new CSpamProtection(cpiPlayer.SoldierName));
				}
			}
			
			return adminsOnline;
		}
		
        public void OnCommandAdmins(string strSpeaker, string strText, MatchCommand mtcCommand, CapturedCommand capCommand, CPlayerSubset subMatchedScope)
        {
            if (!SpamCheck(strSpeaker))
            {		
                if (this.blUseProconAccounts == enumBoolYesNo.Yes || this.blUseReservedSlots == enumBoolYesNo.Yes || this.blUseCustomList == enumBoolYesNo.Yes)
                {
                    this.ExecuteCommand("procon.protected.send", "admin.say", "[Notify Me!] List of admins online:", "player", strSpeaker);
                }
                else
                {
                    this.ExecuteCommand("procon.protected.send", "admin.say", "[Notify Me!] No admins have been defined :(", "player", strSpeaker);
                }

				List<string> adminsOnline = this.AdminsOnline();
				
                foreach (string str in adminsOnline)
                {
                    this.ExecuteCommand("procon.protected.send", "admin.say", "[Notify Me!] " + str, "player", strSpeaker);
                }

                if (adminsOnline.Count > 0)
                {
                    this.ExecuteCommand("procon.protected.send", "admin.say", "[Notify Me!] Number of admins online: " + adminsOnline.Count, "player", strSpeaker);
                }
                else
                {
                    this.ExecuteCommand("procon.protected.send", "admin.say", "[Notify Me!] There are currently no admins ingame :(", "player", strSpeaker);
                }

                if (this.blNotifyConsole == enumBoolYesNo.Yes)
                {
                    this.ConsoleWrite(strSpeaker + " listed all admins currently online.");
                }
                if (this.blNotifyChat == enumBoolYesNo.Yes)
                {
                    this.ChatWrite(strSpeaker + " listed all admins currently online.");
                }
                if (this.blNotifyNotification == enumBoolYesNo.Yes)
                {
                    this.NotificationWrite(strSpeaker + " listed all admins currently online.", "false");
                }
                if (this.blNotifyIngame == enumBoolYesNo.Yes)
                {
                    this.IngameWrite(strSpeaker + " listed all admins currently online.");
                }
            }
        }

        public void OnCommandCallAdmin(string strSpeaker, string strText, MatchCommand mtcCommand, CapturedCommand capCommand, CPlayerSubset subMatchedScope)
        {
			if (this.blRequireReason == enumBoolYesNo.Yes && capCommand.ExtraArguments == String.Empty)
			{
				this.ExecuteCommand("procon.protected.send", "admin.say", "[Notify Me!] Please provide a reason when calling an admin!", "player", strSpeaker);
				return;
			}
		
            if (!SpamCheck(strSpeaker))
            {
                if (this.blUseProconAccounts == enumBoolYesNo.No && this.blUseReservedSlots == enumBoolYesNo.No && this.blUseCustomList == enumBoolYesNo.No)
                {
                    this.ExecuteCommand("procon.protected.send", "admin.say", "[Notify Me!] No admins have been defined :(", "player", strSpeaker);
                }

				List<string> adminsOnline = this.AdminsOnline();
				
                if (adminsOnline.Count == 0 || this.blAlwaysSendMail == enumBoolYesNo.Yes)
                {
                    if (this.blNotifyConsole == enumBoolYesNo.Yes || this.blNotifyChat == enumBoolYesNo.Yes || this.blNotifyNotification == enumBoolYesNo.Yes || this.blNotifyEmail == enumBoolYesNo.Yes)
                    {
                        this.ExecuteCommand("procon.protected.send", "admin.say", "[Notify Me!] A notification has been sent out. Help should arrive soon!", "player", strSpeaker);
                        if (this.blNotifyEmail == enumBoolYesNo.Yes)
                        {
                            this.PrepareEmail(strSpeaker, strText);
                        }
                    }
                    else
                    {
                        this.ExecuteCommand("procon.protected.send", "admin.say", "[Notify Me!] I tried my best, but there is no way I can reach HQ. Sorry about that, Johnny!", "player", strSpeaker);
                    }
                }

                if (this.blNotifyConsole == enumBoolYesNo.Yes)
                {
                    string str = "^b" + strSpeaker + " requested an admin!";
                    if (capCommand.ExtraArguments != String.Empty)
                    {
                        str += " (" + capCommand.ExtraArguments + ")";
                    }
                    this.ConsoleWrite(str);
                }
                if (this.blNotifyChat == enumBoolYesNo.Yes)
                {
                    string str = "^b" + strSpeaker + " requested an admin!";
                    if (capCommand.ExtraArguments != String.Empty)
                    {
                        str += " (" + capCommand.ExtraArguments + ")";
                    }
                    this.ChatWrite(str);
                }
                if (this.blNotifyNotification == enumBoolYesNo.Yes)
                {
                    string str = strSpeaker + " requested an admin!";
                    if (capCommand.ExtraArguments != String.Empty)
                    {
                        str += " (" + capCommand.ExtraArguments + ")";
                    }
                    this.NotificationWrite(str, "true");
                }
                if (this.blNotifyIngame == enumBoolYesNo.Yes)
                {
                    string str = strSpeaker + " requested an admin!";
                    if (capCommand.ExtraArguments != String.Empty)
                    {
                        str += " (" + capCommand.ExtraArguments + ")";
                    }
                    this.IngameWrite(str);
                }
            }
        }

        #endregion

        #region own classes/structs

        private class CSpamProtection
        {
            public string strPlayerName;
            public int iRequestCount;
            public DateTime dtLastRequest;

            public CSpamProtection(string name)
            {
                if (name != String.Empty)
                {
                    this.strPlayerName = name;
                    this.iRequestCount = 0;
                    this.dtLastRequest = DateTime.Now;
                }
            }
        }

        #endregion

        #region used interfaces

        // called when a player first joins the server
        public override void OnPlayerJoin(string strSoldierName)
        {
            if (!this.dicAntiSpam.ContainsKey(strSoldierName))
            {
                this.dicAntiSpam.Add(strSoldierName, new CSpamProtection(strSoldierName));
            }

            this.RegisterAllCommands();
        }

        // called when a player leaves the server
        public override void OnPlayerLeft(CPlayerInfo cpiPlayer)
        {
            if (this.dicAntiSpam.ContainsKey(cpiPlayer.SoldierName))
            {
                this.dicAntiSpam.Remove(cpiPlayer.SoldierName);
            }
			
			if (this.dicPBList.ContainsKey(cpiPlayer.SoldierName))
			{
				this.dicPBList.Remove(cpiPlayer.SoldierName);
			}

            this.RegisterAllCommands();
        }

        // returns a list of all players (using the given subset)
        // every instance of procon calls this method every 30 seconds (> 1 logins -> more calls)
        public override void OnListPlayers(List<CPlayerInfo> lstPlayers, CPlayerSubset cpsSubset)
        {
            if (cpsSubset.Subset == CPlayerSubset.PlayerSubsetType.All)
            {
                this.lstPlayers = lstPlayers;

                this.RegisterAllCommands();
            }
        }

        // called when a player types something into global chat
        public override void OnGlobalChat(string strSpeaker, string strMessage)
        {
            if ((this.blMonitorChat == enumBoolYesNo.Yes) && (strMessage.CompareTo(strPreviousMessage) != 0) && (strSpeaker.CompareTo("Server") != 0))
            {
                string[] line = strMessage.ToLower().Split(new char[] { ' ' });
                foreach (string word in line)
                {
                    foreach (string alert in this.lstKeywordList)
                    {
                        if (word.ToLower().Contains(alert.ToLower()))
                        {
                            if (this.blNotifyConsole == enumBoolYesNo.Yes)
                            {
                                this.ConsoleWrite("^b" + strSpeaker + " used a buzzword (" + word + ")!");
                            }
                            if (this.blNotifyChat == enumBoolYesNo.Yes)
                            {
                                this.ChatWrite("^b" + strSpeaker + " used a buzzword (" + word + ")!");
                            }
                            if (this.blNotifyNotification == enumBoolYesNo.Yes)
                            {
                                this.NotificationWrite(strSpeaker + " used a buzzword (" + word + ")!", "false");
                            }
                            if (this.blNotifyIngame == enumBoolYesNo.Yes)
                            {
                                this.IngameWrite(strSpeaker + " used a buzzword (" + word + ")!");
                            }
                        }
                    }
                }
                this.strPreviousMessage = strMessage;
            }
        }

        // called when a player types something into his team's chat
        public override void OnTeamChat(string strSpeaker, string strMessage, int iTeamID)
        {
            this.OnGlobalChat(strSpeaker, strMessage);
        }

        // called when a player types something into his squad's chat
        public override void OnSquadChat(string strSpeaker, string strMessage, int iTeamID, int iSquadID)
        {
            this.OnGlobalChat(strSpeaker, strMessage);
        }

        public override void OnReservedSlotsList(List<string> lstSoldierNames)
        {
            this.lstReservedSlots = lstSoldierNames;
        }

        public override void OnServerInfo(CServerInfo csiServerInfo)
        {
            this.csiServer = csiServerInfo;
        }

		public override void OnRoundOver(int winningTeamId)
		{
			foreach (KeyValuePair<string, CSpamProtection> kvp in this.dicAntiSpam)
			{
				kvp.Value.iRequestCount = 0;
			}
		}
		
        public override void OnLoadingLevel(string strMapFileName, int roundsPlayed, int roundsTotal)
        {
			foreach (KeyValuePair<string, CSpamProtection> kvp in this.dicAntiSpam)
			{
				kvp.Value.iRequestCount = 0;
			}
        }

        public override void OnAccountLogin(string accountName, string ip, CPrivileges privileges)
        {
        
        }


        public override void OnAccountLogout(string accountName, string ip, CPrivileges privileges)
        {
        
        }
		
		public override void OnPunkbusterPlayerInfo(CPunkbusterInfo playerInfo)
		{
			if (playerInfo != null)
			{
                if (this.dicPBList.ContainsKey(playerInfo.SoldierName) == false)
				{
                    this.dicPBList.Add(playerInfo.SoldierName, playerInfo);
                }
                else
				{
                    this.dicPBList[playerInfo.SoldierName] = playerInfo;
                }
            }
		}

        #endregion
    }
}