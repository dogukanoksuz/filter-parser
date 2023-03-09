project('filter-api', 'cpp')
threads_dep = dependency('threads')
pistache_dep = dependency('libpistache')
curl_dep = dependency('libcurl')

executable('filter-api', 'main.cpp', dependencies: [pistache_dep, threads_dep, curl_dep])