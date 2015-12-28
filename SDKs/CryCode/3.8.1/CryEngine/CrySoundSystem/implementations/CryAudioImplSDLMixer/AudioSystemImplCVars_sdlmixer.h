// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef AUDIOSYSTEMIMPLSDLMIXERCVARS_H_INCLUDED
#define AUDIOSYSTEMIMPLSDLMIXERCVARS_H_INCLUDED

class CAudioSystemImplCVars
{
public:

	CAudioSystemImplCVars();
	~CAudioSystemImplCVars();

	void	RegisterVariables();
	void	UnregisterVariables();

	int		m_nPrimaryPoolSize;

private:

	CAudioSystemImplCVars(CAudioSystemImplCVars const&);						// Copy protection
	CAudioSystemImplCVars& operator=(CAudioSystemImplCVars const&);	// Copy protection
};

extern CAudioSystemImplCVars g_SDLMixerImplCVars;

#endif // AUDIOSYSTEMIMPLSDLMIXERCVARS_H_INCLUDED
