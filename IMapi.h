/*
 *	$Header: /MindProbe/IMapi.h 38    11/03/98 2:10p Admin $
 *
 *	$Log: /MindProbe/IMapi.h $
 * 
 * 38    11/03/98 2:10p Admin
 * Added jackpot and gambit.  Removed random player selection.  Added part
 * of MPede support.  Added response to ChatgamesID request.  Added code
 * to track server name.
 */
class CIMapi
{
public:
					CIMapi();
					~CIMapi();

	enum errorCodes
	{
		IMAPI_SUCCESS = 0,
		IMAPI_LOADFAILED,
		IMAPI_INVALIDDLL,
		IMAPI_FAILTO,
		IMAPI_FAILCC,
		IMAPI_FAILATTACH
	};

//	Attributes
	void			Subject(LPCTSTR subject)	{ m_message.lpszSubject = (LPTSTR) subject; }
	void			Text(LPCTSTR text)			{ m_text = text; }

	UINT			Error();
	void			From(LPCTSTR from)			{ m_from.lpszName = (LPTSTR) from; }

	static BOOL		HasEmail();

//	Operations
	BOOL			To(LPCTSTR recip);
	BOOL			Cc(LPCTSTR recip);
	BOOL			Attach(LPCTSTR path, LPCTSTR name = NULL);
	
	BOOL			Send(ULONG flags = 0);

private:
	BOOL			AllocNewTo();

	MapiMessage		m_message;
	MapiRecipDesc	m_from;
	UINT			m_error;
	CString			m_text;

	ULONG (PASCAL *m_lpfnSendMail)(ULONG, ULONG, MapiMessage*, FLAGS, ULONG);
	
	static HINSTANCE m_hInstMail;
	static BOOL		m_isMailAvail;
};
