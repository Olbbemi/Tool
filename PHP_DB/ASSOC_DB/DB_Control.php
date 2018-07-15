<pre>

<?php

include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LIB/SystemLog_Lib.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LIB/ERROR_Handler.php";

/*
    데이터베이스에 연결하는 함수 [핸들은 out 변수이며 함수의 반환은 true/false]
*/
function DB_Connection(&$p_DB_handle, $p_DB_ip, $p_DB_id, $p_DB_pass, $p_DB_name, $p_DB_port)
{
    $p_DB_handle = mysqli_connect($p_DB_ip, $p_DB_id, $p_DB_pass, $p_DB_name, $p_DB_port);
    if($p_DB_handle === false)
    {
        System_LOG(-1, "", "");
        return false;
    }

    return true;
}

/*
    데이터베이스와 연결을 종료하는 함수 [핸들은 out 변수이며 함수의 반환값 없음]
*/
function DB_Disconnection(&$p_DB_handle)
{
    mysqli_close($p_DB_handle);
}

/*
    데이터베이스에 Select 쿼리문을 실행하는 함수 [핸들과 쿼리성공시 얻는 데이터 집합은 out 변수이며 함수의 반환은 true/false]
*/
function DB_GeneralQuery(&$p_DB_handle, $p_DB_query, &$p_query_result)
{
    $p_query_result = mysqli_query($p_DB_handle, $p_DB_query);
    if($p_query_result === false)
    {
        System_LOG(-1, "", "");
        DB_TranactionRollback($p_DB_handle);
        return false;
    }

    return true;
}

/*
    데이터베이스에 데이터의 변화와 관련된 쿼리문을 실행하는 함수 [핸들과 해당 세션의 최근 AI값, 에러변수는 out 변수이며 함수의 반환은 true/false]
*/
function DB_TransactionQuery(&$p_DB_handle, $p_DB_querys, &$p_recent_AI_id, &$p_errno)
{
    mysqli_begin_transaction($p_DB_handle);
    foreach($p_DB_querys as $query)
    {
        if(is_string($query) === true)
        {
            if(mysqli_query($p_DB_handle, $query) === false)
            {
                //System_LOG(-1, "", "");
                $p_errno = mysqli_errno($p_DB_handle);
                DB_TranactionRollback($p_DB_handle);

                return false;
            }
        }
        else
        {
            System_LOG(-1, "", "");
            DB_TranactionRollback($p_DB_handle);
            return false;
        }
    }

    $p_recent_AI_id = mysqli_insert_id($p_DB_handle);
    mysqli_commit($p_DB_handle);

    return true;
}

/*
    DB_TransactionQuery 함수 실행 중 에러 발생 시 트랜젝션을 롤백하는 함수 [핸들은 out 변수이며 함수의 반환값 없음]
*/
function DB_TranactionRollback(&$p_DB_handle)
{
    mysqli_rollback($p_DB_handle);
}

 ?>

</pre>
