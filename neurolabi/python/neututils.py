def create_text_file(filePath, lines):
    with open(filePath, 'w') as f:
        for line in lines:
            f.write(line + '\n')
        f.close()

