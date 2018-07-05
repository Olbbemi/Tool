#ifndef FrameSkip_Info
#define FrameSkip_Info

class FRAMESKIP
{
private:
	bool m_check, m_is_flip;
	long m_pretime, m_curtime, m_totaltime;

public:
	FRAMESKIP();
	~FRAMESKIP();

	bool IsFrameSkip();
};

#endif
