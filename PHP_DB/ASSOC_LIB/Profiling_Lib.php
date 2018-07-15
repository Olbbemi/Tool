<pre>

<?php
// 조금더 분석이 필요한 파일 분석후 해당 주석 지우기
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LOG/Socket_Http.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LIB/ERROR_Handler.php";

define("PF_TYPE_START", 1);
define("PF_PAGE", 1);          //  전체 페이지 처리
define("PF_MYSQL_CONNECT", 2); // MySQL 연결 처리
define("PF_MYSQL_QUERY", 3);   // MySQL 쿼리 처리
define("PF_EXTAPI", 4);        // 외부 API 처리
define ("PF_LOG", 5);          // 로그 처리
define("PF_TYPE_END", 5);

class Profile
{
    private $LOG_URL = '';
    private $LOG_FLAG = FALSE;

    private $_ACTION = '';
    private $_PROFILING = array();

    private $QUERY = '';
    private $COMMENT = '';

    function __construct()
    {
        $this->_PROFILING[PF_PAGE]['start']  = 0;           $this->_PROFILING[PF_PAGE]['sum']  = 0;
        $this->_PROFILING[PF_MYSQL_CONNECT]['start']  = 0;  $this->_PROFILING[PF_MYSQL_CONNECT]['sum']  = 0;
        $this->_PROFILING[PF_MYSQL_QUERY]['start']  = 0;    $this->_PROFILING[PF_MYSQL_QUERY]['sum']  = 0;
        $this->_PROFILING[PF_EXTAPI]['start']  = 0;         $this->_PROFILING[PF_EXTAPI]['sum']  = 0;
        $this->_PROFILING[PF_LOG]['start']  = 0;            $this->_PROFILING[PF_LOG]['sum']  = 0;
    }

    static function ProfileLogInstance($p_profile_URL, $p_ActionPath, $p_LogRate = 100)
    {
        static $profile_instance;

        if(isset($profile_instance) === false)
            $profile_instance = new Profile();

        if($p_ActionPath != '')
            $profile_instance->_ACTION = $p_ActionPath;

        $profile_instance->LOG_URL = $p_profile_URL;

        if(rand() % 100 < $p_LogRate)
            $profile_instance->LOG_FLAG = true;

        return $profile_instance;
    }

    function BeginProfile($p_Type)
    {
        if($this->LOG_FLAG == false)
            return;

        if($p_Type < PF_TYPE_START || PF_TYPE_END < $p_Type)
            return;

        $this->_PROFILING[$p_Type]['start'] = microtime(true);
    }

    function EndProfile($p_Type, $p_comment = '')
    {
        if($this->LOG_FLAG == false)
            return;

        if($p_Type < PF_TYPE_START || PF_TYPE_END < $p_Type)
            return;

        $endTime = microtime(true);
        $this->_PROFILING[$p_Type]['sum'] += ($endTime - $this->_PROFILING[$p_Type]['start']);

        if($p_Type == PF_MYSQL_QUERY)
            $this->QUERY .= $p_comment."\n";
        else
            $this->COMMENT .= $p_comment."\n";
    }

    function LOG_Save($p_AccountNo = -1)
    {
        if($this->LOG_FLAG == false)
            return;

        $post_string = array(
          "IP"           => $_SERVER['REMOTE_ADDR'],
          "AccountNo"    => $p_AccountNo,
          "Action"       => $this->_ACTION,
          "T_Page"       => $this->_PROFILING[PF_PAGE]['sum'],
          "T_Mysql_Connnection" => $this->_PROFILING[PF_MYSQL_CONNECT]['sum'],
          "T_Mysql_Query"       => $this->_PROFILING[PF_MYSQL_QUERY]['sum'],
          "T_ExtAPI"     => $this->_PROFILING[PF_EXTAPI]['sum'],
          "T_Log"        => $this->_PROFILING[PF_LOG]['sum'],
          "Query"        => $this->QUERY,
          "Comment"      => $this->COMMENT,
                            );

        Request_Http($this->LOG_URL, json_encode($post_string), "POST");
    }
}

?>

</pre>
