#ifndef _H_STRINGS_
#define _H_STRINGS_

// for multiple using strings

const String httpRedirect = "<br/>You will be redirect to main page after 5 seconds<br/><script>$(document).ready(function () { window.setTimeout(function () { location.href = 'http://192.168.0.1'; }, 5000);});<script>";


const String botCmdStart = "/start";
const String botCmdStartDescription = " - начать работу с ботом\r\n";
const String botCmdOpen = "/open";
const String botCmdOpenDescription = " - открыть шлагбаум\r\n";
const String botCmdClose = "/close";
const String botCmdCloseDescription = " - закрыть шлагбаум\r\n";

const String botCmdEditState= "/edit_state";
const String botCmdEditStateDescription = " - изменить статус шлагбаума у бота при рассинхронизации\r\n";
const String botCmdSetOpened = "/set_opened";
const String botCmdSetOpenedDescription = " - шлагбаум сейчас открыт\r\n";
const String botCmdSetClosed = "/set_closed";
const String botCmdSetClosedDescription = " - шлагбаум сейчас закрыт\r\n";

const String botCmdWhitelist = "/whitelist";
const String botCmdWhitelistDescription = " - список пользователей\r\n";
const String botCmdWhitelistEdit = "/whitelist_edit";
const String botCmdWhitelistEditDescription = " - редактировать список пользователей\r\n";
const String botCmdWhitelistAdd = "/whitelist_add";
const String botCmdWhitelistAddDescription = " - добавить пользователя\r\n\r\n";
const String botCmdWhitelistRemove = "/whitelist_remove";
const String botCmdWhitelistRemoveDescription = " - удалить пользователя\r\n\r\n";
const String botCmdWhitelistExit = "/whitelist_exit";
const String botCmdWhitelistExitDescription = " - выйти из редактирования списка пользователей\r\n";

const String botCmdAdminlist = "/adminlist";
const String botCmdAdminlistDescription = " - список администраторов\r\n";
const String botCmdAdminlistEdit = "/adminlist_edit";
const String botCmdAdminlistEditDescription = " - редактировать список администраторов\r\n";
const String botCmdAdminlistAdd = "/adminlist_add";
const String botCmdAdminlistAddDescription = " - добавить администратора\r\n\r\n";
const String botCmdAdminlistRemove = "/adminlist_remove";
const String botCmdAdminlistRemoveDescription = " - удалить администратора\r\n\r\n";
const String botCmdAdminlistExit = "/adminlist_exit";
const String botCmdAdminlistExitDescription = " - выйти из редактирования списка администраторов\r\n";

const String botCmdYes = "/yes";
const String botCmdYesDescription = " - да\r\n";
const String botCmdNo = "/no";
const String botCmdNoDescription = " - нет\r\n";

const String botMsgUserPrefix = "@"; // 1 char
const String botMsgEnterWhitelistUsername = "Введите @userName пользователя. UserName должен начинаться с \"" + botMsgUserPrefix + "\" (если пусто - префикс не требуется).";
const String botMsgEnterAdminlistUsername = "Введите @userName админа. UserName должен начинаться с \"" + botMsgUserPrefix + "\" (если пусто - префикс не требуется).";
const String botMsgCommands = "Команды бота.\r\n";
const String botMsgOpened = "Шлагбаум открыт.\r\n";
const String botMsgClosed = "Шлагбаум закрыт.\r\n";
const String botMsgComplete = "Выполнено.\r\n";

#endif