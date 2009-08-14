from enigma import *
from Screens.Screen import Screen
from Plugins.Plugin import PluginDescriptor

class TuxTermStarter(Screen):
	skin = """
		<screen position="1,1" size="1,1" title="TuxTerm" >
                </screen>"""

	def __init__(self, session, args = None):
		self.skin = TuxTermStarter.skin
		Screen.__init__(self, session)
		self.container=eConsoleAppContainer()
		self.container.appClosed.append(self.finished)
		self.runapp()
		
	def runapp(self):
		eDBoxLCD.getInstance().lock()
		eRCInput.getInstance().lock()
		fbClass.getInstance().lock()
		if self.container.execute("/usr/bin/tuxterm"):
			self.finished(-1)

	def finished(self,retval):
		fbClass.getInstance().unlock()
		eRCInput.getInstance().unlock()
		eDBoxLCD.getInstance().unlock()
		self.close()

def main(session, **kwargs):
	session.open(TuxTermStarter)

def Plugins(**kwargs):
	return PluginDescriptor(name="TuxTerm", description="Terminal for Tuxbox", where = PluginDescriptor.WHERE_PLUGINMENU, fnc=main)
	