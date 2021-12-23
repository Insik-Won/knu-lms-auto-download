■ readme

◎ knu-lms-auto-download에 대해
  - 이 프로젝트는 경북대학교 LMS에 있는 과목의 강의자료를 쉽게 다운로드 받기 위해 만들어진 프로젝트입니다. 
  - 이 프로젝트의 프로그램은 knudown입니다.

◎ 설치방법
  - 아래 명령어를 입력한다.
  - <code>
    git clone https://github.com/Insik-Won/knu-lms-auto-download.git
    chmod +x ./install.sh
    sudo ./install.sh
  - </code>

◎ 사용방법
  - $ knudown [options] [subject_name] [output_directory]
  - $ knudown [options] [subject_name] -D [output_directory]

  - [subject_name]
    - 경북대 LMS에 있는 과목 이름의 일부분
    - 과목 이름이 중복된다면 첫번째로 일치하는 과목을 선택합니다.
  - [output_directory]
    - 강의자료가 다운로드될 디렉토리
  - [options]
    - -U, --userid: 경북대 lms 통합로그인 사용자의 ID
      - 미입력 시, 표준 입력으로 입력합니다.
    - -P, --password: 경북대 lms 통합로그인 사용자의 비밀번호. 비추천
      - 미입력 시, 표준 입력으로 입력합니다. (ECHO 없음)
    - -D, --directory: 강의자료가 다운로드될 디렉토리
    - -t, --posttitle: 다운로드할 강의자료 게시글의 제목의 일부분 
    - -n, --postnum: 다운로드할 강의자료 게시글의 게시글 번호
    - -o, --postorder: 다운로드할 강의자료 게시글을 올라온 순서로 나열한 경우의 번호
      - -t, -n, -o 중 두개 이상이 있다면 가장 마지막 옵션이 적용됩니다.
    - -i, --include: 강의자료 중 다운로드할 파일의 glob 패턴
    - -e, --exclude: 강의자료 중 다운로드하지 않을 파일의 glob 패턴
      - -i가 먼저 적용되고 -e가 다음에 적용됩니다.
    - -q, --quiet: 출력이나 오류를 출력하지 않습니다.
