<pre>

<?php

/*
#SystemLog 테이블 템플릿'DOCUMENT_ROOT'

CREATE TABLE `GameLog_template`
(
    `Index`			bigint(20) NOT NULL AUTO_INCREMENT,
    `date`		  datetime NOT NULL,
    `AccountNo`	bigint(20) NOT NULL,
    `LogType`		int(11) NOT NULL,
    `LogCode`		int(11) NOT NULL,
    `Param1`		int(11) DEFAULT '0',
    `Param2`		int(11) DEFAULT '0',
    `Param3`		int(11) DEFAULT '0',
    `Param4`		int(11) DEFAULT '0',
    `ParamString`	varchar(256) DEFAULT '',
     PRIMARY KEY (`Index`)
) ENGINE=MyISAM;
*/

//------------------------------------------------------------
// 게임 로그 남기는 함수 / DB 및 테이블 정보는 DB_Config.php 참고.
//
// 문자열 인자의 공격 검사는 하지 않음, 내부 서버 IP 외에는 본 파일을 호출하지 못하도록 방화벽에서 차단 되어야 함
// POST 방식으로 로그덩어리 (LogChunk) 에 JSON 포멧으로 들어오면 이를 배열로 풀어서 하나하나 저장한다.
// 게임로그는 하나하나 개별적인 저장이 아닌 컨텐츠 별로 관련 로그를 한방에 모아서 저장한다.
//
// array(	array(MemberNo, LogType, LogCode, Param1, Param2, Param3, Param4, ParamString),
//			array(MemberNo, LogType, LogCode, Param1, Param2, Param3, Param4, ParamString));
//
// 2차원 배열로 한 번에 여러개의 로그가 몰아서 들어옴.
//------------------------------------------------------------

include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_DB/DB_Log_Config.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_DB/DB_Control.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LIB/SystemLog_Lib.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LIB/ERROR_Handler.php";

$count = 0;
$errno = 0; // DB_TransactionQuery 함수에 사용되는 out 변수로 해당 함수내부에서 에러 발생시 확인할 변수
$recent_AI_id; // DB_TransactionQuery 함수에서 사용되는 out 변수로 해당 세션의 최근 AI를 확인할 변수
$query = array();
$Trans_Query = array();
    
$JSON_Data = json_decode(file_get_contents("php://input"), true);
$check = DB_Connection($config_log_DB_game_handle, $config_log_DB_host, $config_log_DB_id, $config_log_DB_pass, $config_log_DB_name, $config_log_DB_port);
if($check === false)
{
    System_LOG(-1, "", "");
    exit;
}

mysqli_set_charset($config_log_DB_game_handle, "utf8");

// JSON 의 배열 개수만큼 반복문을 돌면서 데이터를 뽑아서 INSERT 쿼리 생성
foreach($JSON_Data as $key => $value)
{
    $query[] = "(
                    NULL,
                    NOW(),
                    '{$value['AccountNo']}',
                    '{$value['LogType']}',
                    '{$value['LogCode']}',
                    '{$value['Param1']}',
                    '{$value['Param2']}',
                    '{$value['Param3']}',
                    '{$value['Param4']}',
                    '{$value['ParamString']}'
                )";
    $count++;
}

// 현재날짜 테이블에 로그 삽입
$Table_Name = "GameLog_".date("Ym");
for($i = 0; $i < $count; $i++)
    $Trans_Query[] = "INSERT INTO `{$Table_Name}` (`Index`, `Date`, `AccountNo`, `LogType`, `LogCode`, `Param1`, `Param2`, `Param3`, `Param4`, `ParamString`) VALUES {$query[$i]}";

$Result = DB_TransactionQuery($config_log_DB_game_handle, $Trans_Query, $recent_AI_id, $errno);
if ($Result === false)
{
		// 해당 테이블이 존재하지 않는 경우 생성 후 입력
	if($errno === 1146)
	{
		unset($Trans_Query);
		$Trans_Query[] = "CREATE TABLE `{$Table_Name}` LIKE `gamelog_template`";
        
        for($i = 0; $i < $count; $i++)
        $Trans_Query[] = "INSERT INTO `{$Table_Name}` (`Index`, `Date`, `AccountNo`, `LogType`, `LogCode`, `Param1`, `Param2`, `Param3`, `Param4`, `ParamString`) VALUES {$query[$i]}";
        $Result = DB_TransactionQuery($config_log_DB_game_handle, $Trans_Query, $recent_AI_id, $errno);
	}
	else
		System_LOG(-1, "", "");
}

DB_Disconnection($DB_handle);

?>

</pre>
