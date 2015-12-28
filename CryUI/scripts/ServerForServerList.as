class scripts.ServerForServerList
{
	var m_id:Number;
	var m_ip:String;
	var m_port:Number;
	var m_serverName:String; 
	var m_playersAmmount:String;
	var m_mapName:String; 
	var m_gameRules:String;
	var m_ping:Number;
	var m_InfoField:MovieClip;
	
	
	public function ServerForServerList(_id:Number,_ip:String,_port:Number, _serverName:String, _playersAmmount:String, _mapName:String, _gameRules:String, _ping:Number,_InfoField:MovieClip)
	{
		m_id = _id;
	    m_ip = _ip;
	    m_port = _port;
	    m_serverName = _serverName; 
	    m_playersAmmount = _playersAmmount;
	    m_mapName = _mapName; 
	    m_gameRules = _gameRules;
	    m_ping = _ping;
		
	    m_InfoField = _InfoField;
		m_InfoField.m_id = _id;
		m_InfoField.m_ip = _ip;
		m_InfoField.m_port  = _port;
		m_InfoField.serverName.text = _serverName;
		m_InfoField.playersAmmount.text = _playersAmmount;
		m_InfoField.mapName.text = _mapName; 
		m_InfoField.gameRules.text = _gameRules;
		m_InfoField.ping.text = String(_ping);
	}

	public function Update()
	{
		m_InfoField.serverName.text = m_serverName;
		m_InfoField.playersAmmount.text = m_playersAmmount;
		m_InfoField.mapName.text = m_mapName; 
		m_InfoField.gameRules.text = m_gameRules;
		m_InfoField.ping.text = String(m_ping);
	}
}