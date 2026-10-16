# Nginx 同時提供靜態網頁及 API 反向代理
FROM nginx:stable-alpine
COPY nginx/default.conf /etc/nginx/conf.d/default.conf
COPY web/ /usr/share/nginx/html/
EXPOSE 80
