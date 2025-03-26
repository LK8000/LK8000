cmake_minimum_required(VERSION 3.10)

project(tracking_http)

find_package (curl CONFIG REQUIRED)

add_library (
	${PROJECT_NAME}

	curl_ptr.h
	http_session.h
	http_session.cpp
)

target_link_libraries(
	${PROJECT_NAME}

	curl::curl
)
