cmake_minimum_required(VERSION 3.10)

project(LK8000_GLUTESS)

add_library (
	${PROJECT_NAME}

	dict-list.h
	dict.h
	dict.c
	geom.h
	geom.c
	gluos.h
	glutess.h
	memalloc.h
	memalloc.c
	mesh.h
	mesh.c
	normal.h
	normal.c
	priorityq-heap.h
	priorityq-heap.c
	priorityq-sort.h
	priorityq.h
	priorityq.c
	render.h
	render.c
	sweep.h
	sweep.c
	tess.h
	tess.c
	tessellate.h
	tessellate.c
	tessmono.h
	tessmono.c
)
