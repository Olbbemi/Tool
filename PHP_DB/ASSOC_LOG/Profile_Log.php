<pre>

<?php

/*
#profilingLog 테이블 템플릿

CREATE TABLE `profilingLog_template`
(
		`Index` bigint NOT NULL AUTO_INCREMENT,
		`Date` datetime NOT NULL,
		`IP`	varchar(40) NOT NULL,
		`AccountNo` bigint NOT NULL,
		`Action` varchar(128) DEFAULT NULL,
		`T_Page` FLOAT NOT NULL,
		`T_Mysql_Conn` FLOAT NOT NULL,
		`T_Mysql` FLOAT NOT NULL,
		`T_ExtAPI` FLOAT NOT NULL,
		`T_Log` FLOAT NOT NULL,
		`Query` TEXT NOT NULL,
		`Comment` TEXT NOT NULL,
		 PRIMARY KEY (`Index`)
)	ENGINE=MyISAM DEFAULT CHARSET=utf8
*/

//------------------------------------------------------------
// 프로파일링 로그 남기는 함수 / DB 및 테이블 정보는 DB_Config.php 참고.
//
// 문자열 인자의 공격 검사는 하지 않음, 내부 서버 IP 외에는 본 파일을 호출하지 못하도록 방화벽에서 차단 되어야 함
// $_POST['IP']						: 유저IP
// $_POST['MemberNo']			: 유저번호
// $_POST['Action']				: 액션구분
// $_POST['T_Page']				: Time Page
// $_POST['T_Mysql_Conn']	: Time Mysql Connect
// $_POST['T_Mysql']			: Time Mysql
// $_POST['T_ExtAPI']			: Time API
// $_POST['T_Log']				: Time Log
// $_POST['Query']				: Mysql 이 있는 경우 쿼리문
// $_POST['Comment']			: 그 외 기타 멘트
//------------------------------------------------------------

include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_DB/DB_Log_Config.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_DB/DB_Control.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LIB/SystemLog_Lib.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LIB/ERROR_Handler.php";

$errno = 0; // DB_TransactionQuery 함수에 사용되는 out 변수로 해당 함수내부에서 에러 발생시 확인할 변수
$recent_AI_id; // DB_TransactionQuery 함수에서 사용되는 out 변수로 해당 세션의 최근 AI를 확인할 변수
$Trans_Query = array();

$post_string = file_get_contents("php://input");
$JSON_Data = json_decode($post_string, true);

$IP =  $JSON_Data['IP'];
$AccountNo =  $JSON_Data['AccountNo'];
$Action =  $JSON_Data['Action'];
$T_Page =  $JSON_Data['T_Page'];
$T_Mysql_Connection =  $JSON_Data['T_Mysql_Connection'];
$T_Mysql_Query =  $JSON_Data['T_Mysql_Query'];
$T_ExtAPI =  $JSON_Data['T_ExtAPI'];
$T_Log =  $JSON_Data['T_Log'];
$Query =  $JSON_Data['Query'];
$Comment =  $JSON_Data['Comment'];

$check = DB_Connection($config_log_DB_profile_handle, $config_log_DB_host, $config_log_DB_id, $config_log_DB_pass, $config_log_DB_name, $config_log_DB_port);
if($check === false)
{
    System_LOG(-1, "", "");
	exit;
}

mysqli_set_charset($config_log_DB_profile_handle, "utf8");

// 현재날짜 테이블에 로그 삽입
$Table_Name = "ProfilingLog_".date("Ym");
$Trans_Query[] = "INSERT INTO `{$Table_Name}` (`Index`, `Date`, `IP`, `AccountNo`, `Action`, `T_Page`, `T_Mysql_Conn`, `T_Mysql`, `T_ExtAPI`, `T_Log`, `Query`, `Comment`) VALUES (NULL, NOW(), '{$IP}', '{$AccountNo}', '{$Action}', '{$T_Page}', '{$T_Mysql_Connection}', '{$T_Mysql_Query}', '{$T_ExtAPI}', '{$T_Log}', '{$Query}', '{$Comment}')";

$Result = DB_TransactionQuery($config_log_DB_profile_handle, $Trans_Query, $recent_AI_id, $errno);

// 해당 테이블이 존재하지 않는 경우 생성 후 입력
if ($Result === false)
{
		if($errno == 1146)
		{
				unset($Trans_Query);
				$Trans_Query[] = "CREATE TABLE `{$Table_Name}` LIKE ProfilingLog_template";
				$Trans_Query[] = "INSERT INTO `{$Table_Name}` (`Index`, `Date`, `IP`, `AccountNo`, `Action`, `T_Page`, `T_Mysql_Conn`, `T_Mysql`, `T_ExtAPI`, `T_Log`, `Query`, `Comment`) VALUES (NULL, NOW(), '{$IP}', '{$AccountNo}', '{$Action}', '{$T_Page}', '{$T_Mysql_Connection}', '{$T_Mysql_Query}', '{$T_ExtAPI}', '{$T_Log}', '{$Query}', '{$Comment}')";

				DB_TransactionQuery($config_log_DB_profile_handle, $Trans_Query, $recent_AI_id, $errno);
		}
		else
				System_LOG(-1, "", "");
}

DB_Disconnection($config_log_DB_profile_handle);
?>

</pre>
