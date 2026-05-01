gcc -Wall monitor_reports.c -o monitor_reports
./monitor_reports &
./city_manager --role manager --user alice --add downtown
kill -SIGINT $(cat .monitor_pid)