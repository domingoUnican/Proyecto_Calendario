% tfg_domingo.cls
%
% Fichero de estilo refrito para la memoria de un trabajo de
% fin de grado.

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{tfg_domingo}[2018/01/26 TFG Domingo]

\LoadClass[11pt]{book}

% Contadores absolutos para ilustraciones, ecuaciones y
% tablas.
\RequirePackage{chngcntr}
\counterwithout{figure}{chapter}
\counterwithout{table}{chapter}
\counterwithout{equation}{chapter}

% Tamaño y tipografía
\RequirePackage[a4paper,text={17cm,25cm}]{geometry}
\RequirePackage{fontspec}
\setmainfont[SmallCapsFont={Linux Biolinum Capitals O},
 ItalicFeatures={SmallCapsFont={Linux Biolinum Capitals O/IT}},
 BoldFeatures={SmallCapsFont={Linux Biolinum Capitals O/BF}}]%
 {Century Schoolbook L}
\setmonofont{Terminus (TTF)}

\RequirePackage{xstring}
\RequirePackage{ifthen}
\RequirePackage{xifthen}

% Datos del trabajo
\newcommand\autor[1]{%
  \def\@autor{#1}
  \hypersetup{pdfauthor=#1}
}
\newcommand\titulo[1]{\def\@titulo{#1}}
\newcommand\corto[1]{%
  \ifthenelse{\equal{#1}{} \OR \equal{#1}{ }}
    {\def\@corto{\@titulo}}
    {\def\@corto{#1}}
  \markboth{\@corto}{}
  \hypersetup{pdftitle=#1}
}
\newcommand\ingles[1]{\def\@ingles{#1}}
\newcommand\fecha[1]{\def\@fecha{#1}}
\newcommand\palabras[2]{%
  \def\@palabrasclave{#1}
  \def\@keywords{#2}
}

% Numeración de los capítulos
\RequirePackage{titlesec}
\titleformat{\chapter}[hang]{\normalfont\huge\bfseries}%
  \thechapter{1em}{}

% Hipervínculos
\RequirePackage[colorlinks,allcolors=blue,
                pdfpagelayout=TwoPageRight,
                pdfpagelabels]{hyperref}

% Recuadros
\RequirePackage{tcolorbox}
\tcbset{colback=gray!10}

% Páginas en blanco %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\newcommand\aderecha{%
  \clearpage
  \thispagestyle{empty}
  \cleardoublepage
  \thispagestyle{empty}
}

\def\@schaptersintoc#1{%
  \protected@edef\@currentlabel{#1}%
  \chaptermarksin{#1}%
  \@makeschapterhead{#1}%
  \@afterheading
}

\def\@schapter#1{%
  \@schaptersintoc{#1}
  \phantomsection
  \addcontentsline{toc}{chapter}
  {\protect\numberline{}#1}
  \setcounter{section}{0}
}

\renewcommand\chapter{%
  \aderecha
  \global\@topnum\z@
  \@afterindentfalse
  \secdef\@chapter\@schapter
}

\newcommand\chaptersintoc{%
  \aderecha
  \global\@topnum\z@
  \@afterindentfalse
  \secdef\@schaptersintoc\@schaptersintoc
}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Referencias
\RequirePackage[sort,square]{natbib}
\renewcommand\bibsection{\chapter*{\bibname}}
\bibliographystyle{abbrvnat_trucado}

% Encabezamientos y pies de página
\RequirePackage{fancyhdr}
\pagestyle{fancy}
\fancyfoot{}
\fancyhead{}
\fancyhead[LE]{{\footnotesize\thepage}\hfill\leftmark}
\fancyhead[RO]{\rightmark\hfill{\footnotesize\thepage}}
\renewcommand\chaptermark[1]{\markright{\thechapter\ #1}}
\newcommand\chaptermarksin[1]{\markright{#1}}
\renewcommand\sectionmark[1]{}

% Índice
\renewcommand\tableofcontents{%
  \chaptersintoc*{\contentsname}
  \@starttoc{toc}%
}

% Portada %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\def\fuente{Nimbus Sans L} % Reemplazo para «Arial»
% \def\fuente{Liberation Sans}
\font\fuentea="\fuente" at 14 pt
\font\fuenteb="\fuente/B" at 17 pt
\font\fuentec="\fuente" at 19 pt
\font\fuented="\fuente/BI" at 19 pt
\font\fuentee="\fuente/B" at 20 pt

\newcommand\portada{
  \thispagestyle{empty}
  \thispdfpagelabel{prt}
  \begin{center}
  \includegraphics[width=29.1mm]{UC.png}\vspace{5mm}\par
  {\fuented Facultad de Ciencias}\vspace{2cm}\par
  {\fuentee \@titulo}\vspace{5mm}\par
  {\fuentec \@ingles}\vspace{2cm}\par
  {\fuentea Trabajo de fin de grado\\ para acceder al}
  \vspace{1cm}\par
  {\fuenteb GRADO EN INGENIERÍA INFORMÁTICA}
  \end{center}
  \vfill
  \begin{flushright}
  {\fuentea
  \begin{tabular}{rl}
  Autor:& \@autor\\[3mm]
  Director:& Domingo Gómez Pérez\\[6mm]
  \end{tabular}\par
  \@fecha}
  \end{flushright}
  \clearpage
  \thispagestyle{empty}
  \thispdfpagelabel{vprt}
}

% Agradecimientos %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\newcommand\gracias[1]{%
  \aderecha
  \thispagestyle{empty}
  \hfill{\large\bf \emph{Agradecimientos}}
  \vspace{1cm}\par
  \hfill
  \begin{minipage}{.75\linewidth}
  #1\end{minipage}
}

\newcommand\sucinto[1]{%
  \aderecha
  \thispagestyle{empty}
  \vspace*{4cm}\par
  \hfill\emph{#1}
}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Resúmenes
\newcommand\resumen[2]{%
  \aderecha
  \begin{center}\emph{\large\@titulo}
  \end{center}\vspace{5mm}\par
  {\bf\Large Resumen}
  \hfill\parbox{13cm}%
  {\flushright{\bf palabras clave:} \@palabrasclave}
  \vspace{5mm}\par
  #1\vfill
  \begin{center}
  \rule{.67\linewidth}{1pt}\vspace{5mm}\par
  \emph{\large\@ingles}
  \end{center}\vspace{5mm}\par
  {\bf\Large Abstract}
  \hfill\parbox{13cm}%
  {\flushright{\bf keywords:} \@keywords}
  \vspace{5mm}\par
  #2\vfill
}

% Leyendas
\RequirePackage{caption}

% Código
\RequirePackage{fancyvrb}
\newcommand\codigo[4][]{%
  \begin{minipage}[t]{#4}
  \ifthenelse{\equal{#3}{}}
    {\VerbatimInput[frame=lines,framesep=3mm,%
                    numbers=left,#1]{#2}}%
    {\VerbatimInput[frame=lines,framesep=3mm,%
                    numbers=left,label=#3,#1]{#2}}%
  \end{minipage}\vspace{4mm}
}
\RequirePackage[spanish,ruled]{algorithm2e}

% Otros %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\renewcommand\frontmatter{%
  \aderecha
  \@mainmatterfalse
  \pagenumbering{roman}
}
\renewcommand\mainmatter{%
  \aderecha
  \@mainmattertrue
  \pagenumbering{arabic}
}
\renewcommand\backmatter{%
  \aderecha
  \@mainmatterfalse
}

\RequirePackage{graphicx}
\RequirePackage[spanish]{babel}
\frenchspacing

% Términos en español %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\renewcommand\spanishfigurename{Ilustración}
%\renewcommand\spanishchaptername{} % {Capítulo}
\renewcommand\spanishcontentsname{Índice}
\renewcommand\spanishbibname{Referencias}

\endinput
