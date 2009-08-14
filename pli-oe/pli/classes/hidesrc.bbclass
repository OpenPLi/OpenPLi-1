python populate_packages_append() {
	pn = bb.data.getVar('PN', d)
	pv = bb.data.getVar('PV', d)
	bb.data.setVar('SRC_URI', pn + '-' + pv + '.tar.gz', d)
}
