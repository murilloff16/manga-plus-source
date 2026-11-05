# Tomato para Rakuyomi — experimental

Fonte separada, id `pt.tomato`, para a ABI antiga do Rakuyomi 1.22.2.
Requer o backend adaptado para páginas base64 usado no projeto local do Manga Plus; não funciona com o servidor original sem essa adaptação.

Compilar: `python build_source.py` (Clang com alvo wasm32). O pacote `.aix` não contém credenciais. Configure `token` nas configurações da fonte com uma sessão legítima do Tomato. O login exige captcha e ocorre fora da fonte. Nunca publique o token.

Implementado: catálogo de destaques com deduplicação, lista de capítulos e download de páginas com User-Agent do Tomato. As imagens são retornadas como base64; o token é enviado somente à API do Tomato.

Limitações antes de publicar no índice principal:
- A busca ainda ignora o filtro e retorna apenas o catálogo de destaques.
- Números fracionários de capítulos precisam ser tratados.
- Falhas na API e sessão expirada precisam de melhor tratamento.
- Compilação concluída, mas ainda falta testar o WASM no Rakuyomi e no Kindle físico.
- Memória em capítulos grandes precisa ser verificada no aparelho.

Testes anteriores do serviço, fora do runtime: login com captcha concluído pelo usuário; listagem de One Piece com 1201 registros e números 1–1197 sem lacunas; capítulo 100 com 27 imagens baixadas, decodificadas e CBZ íntegro. Esses testes não substituem a validação da fonte.

Esta implementação está fora dos índices ativos até concluir essas verificações.
