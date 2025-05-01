import React, { useState, useEffect, useCallback } from 'react';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import {
  Table,
  TableBody,
  TableCell,
  TableHead,
  TableHeader,
  TableRow,
} from '@/components/ui/table';
import { Textarea } from '@/components/ui/textarea';
import { Checkbox } from '@/components/ui/checkbox';
import { cn } from '@/lib/utils';
import { motion, AnimatePresence } from 'framer-motion';
//import { ScrollArea } from "@/components/ui/scroll-area" // 제거된 ScrollArea
import { Alert, AlertDescription, AlertTitle } from "@/components/ui/alert"
import { AlertCircle } from "lucide-react"

// PostgreSQL 연결 설정 (electron-store에서 가져오는 방식으로 변경)
//const { Pool } = require('pg'); //  require -> import 로 변경
import { Pool } from 'pg';
//const Store = require('electron-store'); // require -> import 로 변경
import Store from 'electron-store';
const store = new Store();

let pool: any; // 전역 변수로 선언

// Electron 스토어에서 PostgreSQL 연결 문자열을 가져오는 함수
const getConnectionString = () => {
  const connectionString = store.get('pgConnectionString');
  return connectionString;
};

// PostgreSQL 연결 풀을 초기화하는 함수
const initializePool = () => {
    const connectionString = getConnectionString();
    if (connectionString && !pool) {
        try {
            pool = new Pool({
                connectionString: connectionString,
            });
            console.log('PostgreSQL 연결 풀이 초기화되었습니다.');
        } catch (error) {
            console.error('PostgreSQL 연결 풀 초기화 실패:', error);
            pool = null; // 연결 실패 시 pool을 null로 설정
            throw error; // 명시적으로 에러를 던져서 호출하는 쪽에서 처리할 수 있게 함
        }
    } else if (!connectionString) {
        console.warn('PostgreSQL 연결 문자열이 설정되지 않았습니다.');
    }
    return pool;
};

interface Mp3Library {
  id: number;
  title: string;
  artist: string;
  composer: string;
  genre: string;
  genre_1: number;
  genre_2: number;
  genre_3: number;
  album: string;
  track: number;
  release_date: string;
  album_artist: string;
  sample_rate: number;
  duration: string;
  description: string;
  lyrics: string;
  cover_image: Buffer;
  audio_data: Buffer;
  file_name: string;
  file_path: string;
  file_size: number;
  updated_at: string;
}

const Mp3Manager = () => {
  const [mp3Data, setMp3Data] = useState<Mp3Library[]>([]);
  const [selectedColumns, setSelectedColumns] = useState<string[]>(['title', 'artist', 'album', 'genre']);
  const [condition, setCondition] = useState('');
  const [order, setOrder] = useState('');
  const [limit, setLimit] = useState<number | undefined>();
  const [isEditing, setIsEditing] = useState<number | null>(null);
  const [editedData, setEditedData] = useState<Partial<Mp3Library>>({});
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  // 테이블 스키마
    const tableSchema = [
        { column: 'id', label: 'ID', type: 'number' },
        { column: 'title', label: '제목', type: 'text' },
        { column: 'artist', label: '아티스트', type: 'text' },
        { column: 'composer', label: '작곡가', type: 'text' },
        { column: 'genre', label: '장르', type: 'text' },
        { column: 'genre_1', label: '장르 1', type: 'number' },
        { column: 'genre_2', label: '장르 2', type: 'number' },
        { column: 'genre_3', label: '장르 3', type: 'number' },
        { column: 'album', label: '앨범', type: 'text' },
        { column: 'track', label: '트랙', type: 'number' },
        { column: 'release_date', label: '발매일', type: 'date' },
        { column: 'album_artist', label: '앨범 아티스트', type: 'text' },
        { column: 'sample_rate', label: '샘플 속도', type: 'number' },
        { column: 'duration', label: '재생 시간', type: 'text' },
        { column: 'description', label: '설명', type: 'text' },
        { column: 'lyrics', label: '가사', type: 'text' },
        { column: 'cover_image', label: '커버 이미지', type: 'text' }, // Base64로 처리
        { column: 'audio_data', label: '오디오 데이터', type: 'text' }, // Base64로 처리
        { column: 'file_name', label: '파일명', type: 'text' },
        { column: 'file_path', label: '파일 경로', type: 'text' },
        { column: 'file_size', label: '파일 크기', type: 'number' },
        { column: 'updated_at', label: '수정일', type: 'date' },
    ];

  // 데이터 가져오기 함수
  const fetchData = useCallback(async () => {
    setLoading(true);
    setError(null);
    if (!pool) {
        initializePool(); // pool이 초기화되지 않았으면 초기화
    }

    if (!pool) {
      setError("데이터베이스 연결에 실패했습니다. 설정을 확인해주세요.");
      setLoading(false);
      return;
    }

    try {
      // 쿼리 생성
      let query = `SELECT ${['id', ...selectedColumns].join(', ')} FROM mp3_schema.mp3_library`;
      if (condition) {
        query += ` WHERE ${condition}`;
      }
      if (order) {
        query += ` ORDER BY ${order}`;
      }
      if (limit) {
        query += ` LIMIT ${limit}`;
      }
      const client = await pool.connect();
      const result = await client.query(query);
      const data: Mp3Library[] = result.rows;
      setMp3Data(data);
      client.release();
    } catch (err: any) {
        setError(err.message || "데이터를 가져오는 중 오류가 발생했습니다.");
    } finally {
      setLoading(false);
    }
  }, [selectedColumns, condition, order, limit]);

  // 컴포넌트 마운트 시 데이터 가져오기
  useEffect(() => {
    fetchData();
  }, [fetchData]);

    // 수정 핸들러
    const handleEdit = (id: number) => {
        const rowToEdit = mp3Data.find((item) => item.id === id);
        if (rowToEdit) {
            setEditedData(rowToEdit);
            setIsEditing(id);
        }
    };

  // 수정 완료 핸들러
  const handleUpdate = async (id: number) => {
    setLoading(true);
    setError(null);
    if (!pool) {
        initializePool(); // pool이 초기화되지 않았으면 초기화
    }
      if (!pool) {
          setError("데이터베이스 연결에 실패했습니다. 설정을 확인해주세요.");
          setLoading(false);
          return;
      }
    try {
      // 업데이트 쿼리 생성
      const updates: string[] = [];
      const values: any[] = [];
      let valueIndex = 1;
      for (const key in editedData) {
        if (key !== 'id') {
          updates.push(`${key} = $${valueIndex}`);
          values.push(editedData[key]);
          valueIndex++;
        }
      }
      values.push(id); // WHERE 조건에 사용할 id

      const query = `
        UPDATE mp3_schema.mp3_library
        SET ${updates.join(', ')}
        WHERE id = $${valueIndex}
      `;
        const client = await pool.connect();
      await client.query(query, values);
      client.release();

      // 데이터 다시 가져오기
      await fetchData();
      setIsEditing(null);
      setEditedData({});
    } catch (err: any) {
      setError(err.message || "데이터 업데이트 중 오류가 발생했습니다.");
    } finally {
      setLoading(false);
    }
  };

  const handleCancel = () => {
        setIsEditing(null);
        setEditedData({});
    };

  // 컬럼 선택 체크박스 핸들러
    const handleColumnSelect = (column: string) => {
        setSelectedColumns((prevColumns) =>
            prevColumns.includes(column)
                ? prevColumns.filter((c) => c !== column)
                : [...prevColumns, column]
        );
    };

    // 입력 필드 변경 핸들러
    const handleInputChange = (id: number, column: string, value: any) => {
      setEditedData(prev => ({
          ...prev,
          [column]: value
      }));
  };

  return (
    <div className="p-4">
      <h1 className="text-2xl font-bold mb-4">MP3 Library 관리자</h1>

        {/* 설정 오류 메시지 */}
        {error && (
            <Alert variant="destructive" className="mb-4">
                <AlertCircle className="h-4 w-4" />
                <AlertTitle>Error</AlertTitle>
                <AlertDescription>{error}</AlertDescription>
            </Alert>
        )}

      <div className="mb-4">
        <h2 className="text-lg font-semibold mb-2">선택 컬럼:</h2>
        <div className="flex flex-wrap gap-2">
          {tableSchema.map((col) => (
            <div key={col.column} className="flex items-center space-x-2">
              <Checkbox
                id={`col-${col.column}`}
                checked={selectedColumns.includes(col.column)}
                onCheckedChange={() => handleColumnSelect(col.column)}
              />
              <label
                htmlFor={`col-${col.column}`}
                className="text-sm font-medium leading-none peer-disabled:cursor-not-allowed peer-disabled:opacity-70"
              >
                {col.label}
              </label>
            </div>
          ))}
        </div>
      </div>

      <div className="mb-4">
        <h2 className="text-lg font-semibold mb-2">검색 조건 (WHERE):</h2>
        <Input
          type="text"
          value={condition}
          onChange={(e) => setCondition(e.target.value)}
          placeholder="예: artist = 'John Doe'"
          className="max-w-md"
        />
      </div>

      <div className="mb-4">
        <h2 className="text-lg font-semibold mb-2">정렬 순서 (ORDER BY):</h2>
        <Input
          type="text"
          value={order}
          onChange={(e) => setOrder(e.target.value)}
          placeholder="예: title ASC, artist DESC"
          className="max-w-md"
        />
      </div>

      <div className="mb-4">
        <h2 className="text-lg font-semibold mb-2">최대 출력 개수 (LIMIT):</h2>
        <Input
          type="number"
          value={limit || ''}
          onChange={(e) => setLimit(e.target.value ? parseInt(e.target.value, 10) : undefined)}
          placeholder="예: 10"
          className="max-w-md"
        />
      </div>

      <Button onClick={fetchData} disabled={loading} className="mb-4">
        {loading ? 'Loading...' : '데이터 가져오기'}
      </Button>

        {loading && <p className="mb-4">Loading data...</p>}

      {/* 데이터 테이블 */}
      {mp3Data.length > 0 && (
        //<ScrollArea className="h-[500px] w-full rounded-md border"> // 제거된 ScrollArea
          <Table>
            <TableHeader>
              <TableRow>
                <TableHead>수정</TableHead>
                {selectedColumns.map((col) => (
                  <TableHead key={col}>{tableSchema.find(c => c.column === col)?.label || col}</TableHead>
                ))}
              </TableRow>
            </TableHeader>
            <TableBody>
              <AnimatePresence>
                {mp3Data.map((item) => (
                  <motion.tr
                    key={item.id}
                    initial={{ opacity: 0, y: -10 }}
                    animate={{ opacity: 1, y: 0 }}
                    exit={{ opacity: 0, y: 10 }}
                  >
                    <TableCell>
                      {isEditing === item.id ? (
                        <>
                          <Button
                            size="sm"
                            onClick={() => handleUpdate(item.id)}
                            disabled={loading}
                            className="mr-1"
                          >
                            저장
                          </Button>
                          <Button
                            size="sm"
                            variant="outline"
                            onClick={handleCancel}
                            disabled={loading}
                          >
                            취소
                          </Button>
                        </>
                      ) : (
                        <Button
                          size="sm"
                          onClick={() => handleEdit(item.id)}
                          disabled={loading}
                        >
                          수정
                        </Button>
                      )}
                    </TableCell>
                    {selectedColumns.map((col) => {
                        const columnSchema = tableSchema.find(c => c.column === col);
                        const cellValue = item[col as keyof Mp3Library];
                        const displayValue = columnSchema?.type === 'date'
                            ? new Date(cellValue as string).toLocaleDateString()
                            : columnSchema?.type === 'number'
                            ? Number(cellValue).toLocaleString()  // Add thousand separator
                            : cellValue;

                        return (
                            <TableCell key={`${item.id}-${col}`}>
                                {isEditing === item.id ? (
                                    columnSchema?.type === 'text' ? (
                                        <Input
                                            type="text"
                                            value={editedData[col as keyof Mp3Library] ?? cellValue as string}
                                            onChange={(e) => handleInputChange(item.id, col, e.target.value)}
                                            className="w-full"
                                        />
                                    ) : columnSchema?.type === 'number' ? (
                                        <Input
                                            type="number"
                                            value={editedData[col as keyof Mp3Library] ?? cellValue as number}
                                            onChange={(e) => handleInputChange(item.id, col, Number(e.target.value))}
                                            className="w-full"
                                        />
                                    ) : columnSchema?.type === 'date' ? (
                                        <Input
                                            type="date"
                                            value={editedData[col as keyof Mp3Library] ?? (cellValue as string).slice(0, 10)}
                                            onChange={(e) => handleInputChange(item.id, col, e.target.value)}
                                            className="w-full"
                                        />
                                    ) : (
                                        <span>{displayValue}</span>
                                    )
                                ) : (
                                    <span>{displayValue}</span>
                                )}
                            </TableCell>
                        );
                    })}
                  </motion.tr>
                ))}
              </AnimatePresence>
            </TableBody>
          </Table>
        //</ScrollArea> // 제거된 ScrollArea
      )}
    </div>
  );
};

export default Mp3Manager;

