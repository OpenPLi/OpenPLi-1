#this class fills PR with the highest committed svn rev, found in VERSION_URI

python __anonymous () {
	from   bb import data

	src_uri = bb.data.getVar('VERSION_URI', d, 1)
	urls = src_uri.split()
	newurls = []

	def getSvnInfo(svncmd):
		tmppipe = os.popen(svncmd)
		output = tmppipe.readlines()

		infourl = ''
		inforev = ''
		infolastrev = ''
		for line in output:
			if "URL: " in line:
				words = line.split()
				if len(words):
					infourl = words[-1]
			if "Revision: " in line:
				words = line.split()
				if len(words):
					inforev = words[-1]
			if "Last Changed Rev: " in line:
				words = line.split()
				if len(words):
					infolastrev = words[-1]
		return inforev, infolastrev, infourl

	newpr = 0
	for loc in urls:
		(type, host, path, user, pswd, parm) = bb.decodeurl(data.expand(loc, d))
		if type in ['svn']:
			if "module" in parm:
					module = parm["module"]
			else:
					module = ""

			if "rev" in parm:
					revision = parm["rev"]
			else:
					revision = ""

			if "proto" in parm:
					proto = parm["proto"]
			else:
					proto = "svn"

			svn_rsh = None
			if proto == "svn+ssh" and "rsh" in parm:
					svn_rsh = parm["rsh"]

			svnroot = host + path
			svnurl = "%s://%s/%s" % (proto, svnroot, module)

			if revision:
				svncmd = "svn info -r %s %s" % (revision, svnurl)
			else:
				svncmd = "svn info %s" % (svnurl)

			if svn_rsh:
				svncmd = "svn_RSH=\"%s\" %s" % (svn_rsh, svncmd)

			rev = ''

			(inforev, infolastrev, infourl) = getSvnInfo(svncmd)
			rev = infolastrev

			if rev != revision:
				#make sure the url existed at the time of our revision
				revision = rev
				svncmd = "svn info -r %s %s" % (revision, svnurl)
				if svn_rsh:
					svncmd = "svn_RSH=\"%s\" %s" % (svn_rsh, svncmd)

				(inforev, infolastrev, infourl) = getSvnInfo(svncmd)
				rev = infolastrev

			#the new PR should equal the highest rev we found
			if len(rev) and int(rev) > newpr:
				newpr = int(rev)

			if infourl != svnurl:
				#if the url didn't exist yet for our revision,
				#don't use the 'last committed' revision
				#in the checkout command, or we'll get the wrong branch!
				rev = ''

			# Replace the given rev in the url by the last committed rev, which we've just found.
			# This avoids us checking out the same rev over and over again, when nothing changed
			urlparts = loc.split(';')
			newparts = []
			for part in urlparts:
				if "rev=" in part:
					if rev is not '':
						newparts.append("rev=" + rev)
					else:
						newparts.append("rev=HEAD")
				else:
					newparts.append(part)
			loc = ';'.join(newparts)

		newurls.append(loc)

	if newpr:
		this_machine = bb.data.getVar('MACHINE', d, 1)
		prname = 'PR_' + this_machine
		pr = bb.data.getVar(prname, d, 1)
		if not pr or pr == '':
			prname = 'PR'
			pr = bb.data.getVar(prname, d, 1)
		prval = 'r' + str(newpr)
		if pr and pr != '' and pr != 'r0':
			prval = prval + pr
		bb.data.setVar(prname, prval, d)
}
